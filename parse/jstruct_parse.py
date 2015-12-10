import sys

from pycparser import parse_file, c_parser, c_generator, c_ast
import re


class NodeList():
    def __init__(self, other):
        self.idx = 0;
        self._list = list(other)

    def insert(self, other):
        self._list.insert(self.idx, other)
        self.idx = self.idx + 1

    def splice(self, other):
        for o in other:
            self.insert(o)

    def __iter__(self):
        for i in self._list:
            yield i


class ExpansionError(Exception):
    def __init__(self, annotation, node, message=None):
        self.annotation = annotation
        self.node = node
        self.message = message
    def __str__(self):
        message = ''
        if self.message:
            message = ': ' + self.message
        return 'Unable to process annotation @{0} at line {1}{2}\n{3}\n{4}'.format(
            self.annotation['name'],
            self.annotation['line'],
            message,
            repr(self.annotation),
            self.node.show() if hasattr(self.node, 'show') else repr(self.node)
        )

class Annotations():

    PROPERTIES_NAME = '{0}__jstruct_properties__'
    """
    Parse and apply annotations.
    """
    def __init__(self, filename=None):
        if filename:
            self.parse(filename)
        else:
            self.annotations = None
            self.idx = None
            self.len = None

    def get(self, line):
        """
        Return a generator of all
        annotations for the specified line.
        line must be larger than any previous call,
        except after calling parse() or reset()
        """
        a = True
        while a:
            a = self.get_next(line)
            if a:
                yield a

    def get_next(self, line):
        if self.idx >= self.len:
            return None

        if self.annotations[self.idx]['line'] <= line:
            a = self.annotations[self.idx]
            self.idx = self.idx + 1
            return a
        else:
            return None

    # general seek() instead ?
    def reset(self):
        """
        reset the internal index to start pulling
        annotations from the start.
        """
        self.idx = 0

    def get_property_init_list(self, struct):
        """
        Create an InitList describing the property list for a struct
        """

        def make_prop_init_list(decl):
            """
            Create an InitList to instantiate a struct which describes a single property
            """
            taken = []
            exprs = []

            for a in self.get(decl.coord.line):
                name = a['name']
                # skip private properties
                if name == 'private':
                    pass

                init_name, expr = (None, None)
                # append the contents of these annotations directly
                if name in ['schema', 'name']:
                    taken.append(name)
                    init_name = c_ast.ID(name)
                    expr = c_ast.Constant('string', a['content'])

                if init_name is None or expr is None:
                    raise ExpansionError(a, decl, 'Unexpected annotation')

                exprs.append(c_ast.NamedInitializer([init_name], expr))
            if not 'name' in taken:
                exprs.append([
                    c_ast.NamedInitializer([
                        c_ast.ID('name'),
                        c_ast.Constant('string', decl.type.declname)
                    ])
                ])

            return c_ast.InitList(exprs)

        return c_ast.InitList([make_prop_init_list(p) for p in struct.decls])

    def expand(self, ast):
        """
        Expand a pycparser ast with extra structures/data etc
        """
        idx = 0

        struct_object_property_decl = c_ast.Struct('jstruct_object_property', None)

        def ppdirective(a, n, ext):
            ext.insert(c_ast.ID('#{0} {1}'.format(a['directive'], a['content'])))
            return False

        def annotate_struct(a, n, ext):
            name = n.type.name
            struct = n.type
            properties = c_ast.Decl(
                self.PROPERTIES_NAME.format(name),
                [], #quals
                [], #storage
                [], #funcspec
                # type
                c_ast.ArrayDecl(
                    c_ast.TypeDecl(
                        self.PROPERTIES_NAME.format(name),
                        [],
                        struct_object_property_decl
                    ),
                    None, # dim
                    [] # dim_quals
                ),
                self.get_property_init_list(struct), # init
                None # bitsize
            )
            ext.insert(properties)
            return True

        process = {
            '#': ppdirective,
            'json': annotate_struct
        }
        ext = NodeList(ast.ext)
        for n in ast.ext:
            done = False
            annotations = self.get(n.coord.line)
            for a in annotations:
                if not done and a['name'] in process:
                    done = process[a['name']](a, n, ext)
                    if done:
                        try:
                            a_name = a['name']
                            a = next(annotations)
                            raise ExpansionError(
                                a, n, 'Unexpected annotation after ' + a_name)
                        except Exception as e:
                            pass
                else:
                    raise ExpansionError(a, n, 'Unexpected annotation')

        ast.ext = ext

    def parse(self, filename):
        source = ''
        with open(filename, 'r') as infile:
            source = infile.read()
        ANNOTATION_NAME = r'[a-zA-Z_]+'

        # lex + yacc and i'm using a regex? HERESY!!
        annotation_expr = re.compile(
            r'(?:' +
            # match newlines (so we can count them)
            r'(?P<nl>\n)|' +
            # match preprocessor statements
            r'((?!^|\n)#(?P<ppname>define|ifn?def|endif|include)' +
            # match preprocessor statement contents including line continuations
            r'(?:\s+(?P<ppcontent>.*?(?:\\\n.*?)*))?(?=\n|$))|' +
            # match oneline comments
            r'//[\n\s]*@(?P<olname>' + ANNOTATION_NAME + r')|' +
            # oneline annotation content
            r'(?P<olcontent>.*)' +
            # match the entire multiline comment for line counting purposes
            r'/\*(?P<mlwhole>(?:[\n\s])*?@' +
            # match annotation name
            r'(?P<mlname>' + ANNOTATION_NAME + r')[\s\n]*' +
            # match everything after the @annotation in the comment
            r'(?P<mlcontent>(?:\n|.)*?)'+
            # end of multiline comment and non-capturing group
            r')\*/)')
        annotations = []
        line = 1
        match = True
        pos = 0

        while match:
            match = annotation_expr.search(source, pos)

            if not match:
                break

            pos = match.end()
            ppname = match.group('ppname')
            olname = match.group('olname')
            mlname = match.group('mlname')
            if ppname:
                name = '#'
            else:
                name = olname or mlname
            #content = match.group('ppcontent') or match.group('mlwhole')
            content = match.group('mlwhole')
            linecount = content.count('\n') if content else 0

            if match.group('nl'):
                line = line + 1
            elif name:
                if not ppname:
                    content = match.group('olcontent') if olname else match.group('mlcontent')
                annotations.append({
                    'line': line,
                    'lineEnd': line + linecount,
                    'name': name,
                    'content': content,
                    'directive': ppname
                })
            else:
                break

            line = line + linecount

        self.len = len(annotations)
        self.idx = 0

        self.annotations = annotations


def parse_jstruct(filename):
    parser = c_parser.CParser()
    ast = parse_file(filename, use_cpp=True);
    generator = c_generator.CGenerator()

    return (ast, generator)


def generate(ast, generator, annotations):
    decls = [d for d in ast.ext if isinstance(d.type, c_ast.Struct)]

    print('****DECLARATIONS****')
    lastline = 0
    from itertools import chain
    for struct in decls:
        line = struct.type.coord.line

        for a in annotations.get(line):
            print('@' + repr(a))

        print('%s: %s %s' % (line, struct.type.__class__.__name__, struct.type.name))
        for decl in struct.type.decls:
            line = decl.type.coord.line

            for a in annotations.get(line):
                print('@@' + repr(a))
            print(line)
            decl.show()


def prune_ast(ast, filename):
    """
    prune the ast which will contain all the definitions of all the included headers tool
    """
    ast.ext = [e for e in ast.ext if e.coord.file == filename]


def parse_and_generate(filename, outfilename=None):
    from os import path

    annotations = Annotations(filename)

    ast, generator = parse_jstruct(filename)

    prune_ast(ast, filename)

    annotations.expand(ast)

    result = generator.visit(ast)
    print(repr(annotations.annotations))

    if outfilename:
        with open(outfilename, 'w') as outfile:
            outfile.write(result)

    return result

if __name__ == '__main__':
    import os
    try:
        os.mkdir('tests/data/.test')
    except os.OSError:
        pass
    parse_and_generate('tests/data/basic.jstruct.h', 'tests/data/.test/basic.h')
