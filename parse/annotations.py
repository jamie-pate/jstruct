import sys
import StringIO
import re
from pycparser import c_ast

def type_find(decl, ast_type):
    result = decl.type
    while result is not None and not isinstance(result, ast_type):
        if not hasattr(result, 'type'):
            raise ExpansionError(
                None, decl,
                'Unable to find child of type ' + ast_type.__name__
            )
        result = result.type
    return result

def idtype_or_struct_find(decl):
    """
    Find the IdentifierType or Struct inside a Decl. Count PtrDecls as dereference.
    Returns a tuple of (idtype, structtype, dereference)
    """
    idtype = decl.type
    structtype = None
    dereference = 0
    while idtype and not isinstance(idtype, c_ast.IdentifierType):
        if isinstance(idtype, c_ast.PtrDecl):
            dereference = dereference + 1
        if isinstance(idtype, c_ast.Struct):
            structtype = idtype
            idtype = None
            break
        if not hasattr(idtype, 'type'):
            raise ExpansionError(None, decl, 'Could not find IdentifierType or Struct')
        idtype = idtype.type
    return idtype, structtype, dereference


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
        node_show = ''
        if hasattr(self.node, 'show'):
            show_buf = StringIO.StringIO()
            self.node.show(buf=show_buf)
            node_show = str(self.node.coord) + '\n'
            node_show += show_buf.getvalue()
        else:
            node_show = repr(self.node)

        try:
            return 'Unable to process annotation @{0} at line {1}{2}\n{3}\n{4}'.format(
                self.annotation['name'] if self.annotation else None,
                self.annotation['line'] if self.annotation else None,
                message,
                'Annotation: ' + repr(self.annotation),
                'Node: ' + node_show
            )
        except Exception as ex:
            return str(ex)

class Annotations():

    PROPERTIES_NAME = '{0}__jstruct_properties__'.format
    JSTRUCT_TYPE_NAME = 'jstruct_type__{0}__'.format
    """
    Parse and apply annotations.
    """
    def __init__(self, source):
        if source:
            self.parse(source)
        else:
            self.annotations = None
            self.idx = None
            self.len = None
        self._ast_info = None

    def get(self, line):
        """
        Return a generator of all
        annotations for the specified line.
        line must be larger than any previous call,
        except after calling parse() or reset()
        """
        print('GET ANNOTATIONS BEFORE: {0}'.format(line))
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

    def _extract_ast_info(self, ast):
        """
        Extract type info from the ast that can help to statically
        check the validity of annotation expansion
        """
        keys = ('jstruct_extra_type', )
        nodes = (n for n in ast.ext if n.name in keys)

        def extract(n):
            if isinstance(n.type.type, c_ast.Enum):
                return set((e.name for e in n.type.type.values.enumerators))
            raise ExpansionError(None, n, 'Unable to extract useful info')

        self._ast_info = {n.name: extract(n) for n in nodes}

    def get_types(self, decl, type_annotations):
        """
        get the proper types for json, member and jstruct types
        returns a dict with some or all of these members:
        {
            'json': 'json_type_x',
            'member': 'json_type_x',
            'extra': 'jstruct_extra_type_x',
            'jstruct': 'jstruct_type__x__'
        }
        """
        # one-to-one json_types
        json_type_map = {
            'bool': 'bool',
            'int': 'int',
            'double': 'double',
            'char*': 'string'
        }
        # types found here will have a jstruct_extra_type
        # assigned as well as the json_type
        # TODO: generate these instead?
        extra_type_map = {
            'uint32_t': 'int',
            'int64_t': 'int',
            'uint64_t': 'int',
        }

        idtype, structtype, dereference = idtype_or_struct_find(decl)

        def try_get_types(dereference):
            if idtype:
                ctype = ' '.join(idtype.names) + ('*' * dereference)
                result = {}
                try:
                    result['json'] = json_type_map[ctype]
                except KeyError:
                    try:
                        result['json'] = extra_type_map[ctype]
                    except KeyError:
                        raise ExpansionError(
                            type_annotations.get('array', None),
                            decl,
                            '\nUnable to map {0} to json type\n'.format(ctype)
                        )
                    result['extra'] = 'jstruct_extra_type_' + '_'.join(idtype.names)

                    if result['extra'] not in self._ast_info['jstruct_extra_type']:
                        raise ExpansionError(
                            None,
                            decl,
                            result['extra'] + ' is not defined'
                        )
                return result
            else:
                jstruct_type = self.JSTRUCT_TYPE_NAME(structtype.name)
                # struct type
                if dereference == 0:
                    return {
                        'json': 'json_type_object',
                        'jstruct': jstruct_type
                    }
                elif dereference == 1:
                    return {
                        'json': 'json_type_array',
                        'member': 'json_type_object',
                        'jstruct': jstruct_type
                    }
                else:
                    ExpansionError(
                        None,
                        decl,
                        'Unable to deal with property of type {0}{1}'
                        .format('*' * dereference, structtype.name)
                    )

        initial_err = None
        while dereference >= 0:
            try:
                return try_get_types(dereference)
            except ExpansionError as err:
                dereference -= 1
                print(err)
                if initial_err is None:
                    initial_err = err

        raise initial_err

    def get_property_init_list(self, struct):
        """
        Create an InitList describing the property list for a struct
        """

        def make_prop_init_list(decl):
            """
            Create an InitList to instantiate a struct which describes a single property
            decl is the current property c_ast.Decl
            """
            taken = []
            exprs = []
            prop_name = None
            type_annotations = {}

            annotations = self.get(decl.coord.line)
            for a in annotations:
                name = a['name']
                taken.append(name)
                # skip private properties
                if name == 'private':
                    a = next(annotations, None)
                    if a is not None:
                        raise ExpansionError(a, decl, 'Unexpected annotation after ' + name)
                    return

                init_name, expr = (None, None)
                # append the contents of these annotations directly
                if name in ['schema', 'name']:
                    init_name = c_ast.ID(name)
                    if name == 'name':
                        prop_name = name
                    if a['content'] == None:
                        raise ExpansionError(a, decl, 'Content is None')
                    expr = c_ast.Constant('string', a['content'])

                if name in ['nullable']:
                    init_name = c_ast.ID(name)
                    expr = c_ast.Constant('int', '1')

                if name in ['array']:
                    type_annotations[name] = a
                    continue

                if init_name is None or expr is None:
                    raise ExpansionError(a, decl, 'Unexpected annotation')

                exprs.append(c_ast.NamedInitializer([init_name], expr))

            # name the property if it hasn't already been named
            if not 'name' in taken:
                name = type_find(decl, c_ast.TypeDecl).declname
                exprs.append(
                    c_ast.NamedInitializer(
                        [c_ast.ID('name')],
                        c_ast.Constant('string', name)
                    )
                )

            # assign types
            types = [c_ast.NamedInitializer(
                [c_ast.ID(ttype)],
                c_ast.ID(t)
            ) for ttype, t in self.get_types(decl, type_annotations).iteritems()]
            exprs.append(c_ast.NamedInitializer(
                [c_ast.ID('type')],
                c_ast.InitList(types)
            ))
            # calculate struct offset
            exprs.append(c_ast.NamedInitializer(
                [c_ast.ID('offset')],
                c_ast.FuncCall(c_ast.ID('offsetof'), c_ast.ExprList([
                    c_ast.Typename(None, [],
                        c_ast.TypeDecl(None, [],
                            c_ast.Struct(struct.name, None))
                    ),
                    c_ast.ID(name)
                ]))
            ))

            return c_ast.InitList(exprs)

        return c_ast.InitList([make_prop_init_list(p) for p in struct.decls])

    def expand(self, ast, filename):
        """
        Expand a pycparser ast with extra structures/data etc
        """
        idx = 0

        self._extract_ast_info(ast)

        struct_object_property_decl = c_ast.Struct('jstruct_object_property', None)

        def ppdirective(a, n, ext):
            ext.insert(c_ast.ID('#{0} {1}'.format(a['directive'], a['content'])))
            return False

        def annotate_struct(a, n, ext):
            if not isinstance(n.type, c_ast.Struct):
                ExpansionError(a, n,
                    'Cannot expand annotation @{0} on {1}'
                    .format(a['name'], n.__class__.__name__))

            name = n.type.name
            struct = n.type
            properties = c_ast.Decl(
                self.PROPERTIES_NAME(name),
                [], #quals
                [], #storage
                [], #funcspec
                # type
                c_ast.ArrayDecl(
                    c_ast.TypeDecl(
                        self.PROPERTIES_NAME(name),
                        [],
                        struct_object_property_decl
                    ),
                    None, # dim
                    [] # dim_quals
                ),
                self.get_property_init_list(struct), # init
                None # bitsize
            )
            print(properties)
            ext.insert(properties)
            return True

        process = {
            '#': ppdirective,
            'json': annotate_struct
        }
        ext = NodeList(ast.ext)
        for n in ast.ext:
            done = False
            if n.coord.file != filename:
                continue

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

    def parse(self, source):
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
