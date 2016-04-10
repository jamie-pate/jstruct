import sys
import StringIO
import re
import json
from pycparser import c_ast
from pycparser.plyparser import Coord
from inspect import currentframe, getframeinfo


def str_literal(str):
    """
    Escape a string into a c string literal
    """
    # using json.dumps for cheap escaping, it's not going to be perfect
    # TODO: Probably should be improved
    return json.dumps(str)


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
    arraydecl is an ArrayDecl is present
    Returns a tuple of (idtype, structtype, enumtype, arraydecl, dereference)
    """
    idtype = decl.type
    structtype = None
    enumtype = None
    arraydecl = None
    dereference = 0
    while idtype and not isinstance(idtype, c_ast.IdentifierType):
        if isinstance(idtype, c_ast.PtrDecl):
            dereference = dereference + 1
        if isinstance(idtype, c_ast.ArrayDecl):
            arraydecl = idtype
        if isinstance(idtype, c_ast.Struct):
            structtype = idtype
            idtype = None
            break
        if isinstance(idtype, c_ast.Enum):
            enumtype = idtype
            idtype = None
            break
        if not hasattr(idtype, 'type'):
            raise ExpansionError(None, decl, 'Could not find IdentifierType or Struct')
        idtype = idtype.type
    return idtype, structtype, enumtype, arraydecl, dereference


class NodeList():
    def __init__(self, other):
        self.idx = 0;
        self._list = list(other)

    def seek(self, other):
        """
        Place the cursor directly after other in the list
        Return True if other was found, False otherwise.
        Only goes forward
        """
        i = max(0, self.idx - 1)
        length = len(self._list)
        while i < len:
            if self._list[i] == other:
                self.idx = i + 1
                return True
            i += 1
        return False

    def insert(self, other):
        """
        Insert at the current cursor position.
        """
        self._list.insert(self.idx, other)
        self.idx = self.idx + 1

    def splice(self, other):
        """
        insert many at the current cursor position
        """
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

    # TODO: make these configurable?
    PROPERTIES_NAME = '{0}__jstruct_properties__'.format
    JSON_TYPE_NAME = 'json_type_{0}'.format
    LENGTH_PROPERTY_NAME = '{0}__length__'.format
    NULLABLE_PROPERTY_NAME = '{0}__null__'.format
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
            'jstruct': 'x__jstruct_properties__'
        }
        and an ArrayDecl if one was found
        """
        # one-to-one json_types
        json_type_map = {
            'bool': 'boolean',
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

        idtype, structtype, enumtype, arraydecl, dereference = idtype_or_struct_find(decl)
        is_array = arraydecl is not None

        def try_get_types(deref):
            if idtype:
                while deref > -1:
                    ctypename = ' '.join(idtype.names)
                    ctype = ctypename + ('*' * deref)
                    result = {}
                    try:
                        result['json'] = Annotations.JSON_TYPE_NAME(json_type_map[ctype])
                    except KeyError:
                        try:
                            result['json'] = Annotations.JSON_TYPE_NAME(extra_type_map[ctype])
                        except KeyError:
                            raise ExpansionError(
                                type_annotations.get('array', None),
                                decl,
                                '\nUnable to map {0} to json type\n'.format(ctype)
                            )
                        result['extra'] = 'jstruct_extra_type_' + ctypename

                        if result['extra'] not in self._ast_info['jstruct_extra_type']:
                            raise ExpansionError(
                                None,
                                decl,
                                result['extra'] + ' is not defined'
                            )
                    return result
            elif enumtype:
                result = {}
                result['json'] = Annotations.JSON_TYPE_NAME(json_type_map['int'])
                result['extra'] = 'jstruct_enum_extra_type(enum {0})'.format(enumtype.name)
                return result
            else:
                jstruct_type = self.PROPERTIES_NAME(structtype.name)
                # struct type
                if arraydecl:
                    deref += 1
                if deref == 0:
                    return {
                        'json': 'json_type_object',
                        'jstruct': jstruct_type
                    }
                elif deref == 1:
                    return {
                        'json': 'json_type_array',
                        'member': 'json_type_object',
                        'jstruct': jstruct_type
                    }
                else:
                    ExpansionError(
                        None,
                        decl,
                        'Unable to deal with property of type {0}{1}{2}'
                        .format(
                            '*' * dereference,
                            structtype.name,
                            '[]' if arraydecl else ''
                        )
                    )

        initial_err = None
        while dereference >= 0:
            try:
                types = try_get_types(dereference)

                if is_array and arraydecl is None:
                    types['member'] = types['json']
                    types['json'] = 'json_type_array'
                return (types, arraydecl)
            except ExpansionError as err:
                # try again in case it's an array
                if not is_array:
                    dereference -= 1
                    is_array = True
                    if initial_err is None:
                        initial_err = err

        raise initial_err

    def get_property_init_list(self, struct):
        """
        Create an InitList describing the property list for a struct
        Also create a list of c_ast.Decl to append to the struct decls
        """

        def make_extra_decl(name, t):
            idtype = c_ast.IdentifierType([t])
            td = c_ast.TypeDecl(name, [], idtype)
            return c_ast.Decl(
                name,
                [], # quals
                [], # storage
                [], # funcspec
                td, # type
                None, # init
                None, # bitsize
            )

        def make_prop_init_list(decl, extra_decls):
            """
            Create an InitList to instantiate a struct which describes a single property
            decl is the current property c_ast.Decl
            """
            taken = []
            exprs = []
            json_name = None
            nullable = False
            type_annotations = {}

            annotations = self.get(decl.coord.line)
            for a in annotations:
                name = a['name']
                if name == '#':
                    continue
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
                        json_name = a['content']
                    if a['content'] == None:
                        raise ExpansionError(a, decl, 'Content is None')
                    expr = c_ast.Constant('string', str_literal(a['content']))
                    


                if name in ['nullable']:
                    init_name = c_ast.ID(name)
                    expr = c_ast.Constant('int', '1')
                    extra_decls[Annotations.NULLABLE_PROPERTY_NAME(decl.name)] = 'bool'
                    nullable = True

                if name in ['array']:
                    type_annotations[name] = a
                    continue

                if init_name is None or expr is None:
                    raise ExpansionError(a, decl, 'Unexpected annotation')

                exprs.append(c_ast.NamedInitializer([init_name], expr))

            prop_name = type_find(decl, c_ast.TypeDecl).declname

            # name the property if it hasn't already been named
            if json_name is None:
                json_name = prop_name
                exprs.append(
                    c_ast.NamedInitializer(
                        [c_ast.ID('name')],
                        c_ast.Constant('string', str_literal(json_name))
                    )
                )

            # assign types
            types, arraydecl = self.get_types(decl, type_annotations)
            type_inits = [c_ast.NamedInitializer(
                [c_ast.ID(ttype)],
                c_ast.ID(t)
            ) for ttype, t in types.iteritems()]
            fi = getframeinfo(currentframe())
            exprs.append(c_ast.NamedInitializer(
                [c_ast.ID('type')],
                c_ast.InitList(type_inits, Coord(fi.filename, fi.lineno))
            ))
            # calculate struct offset
            exprs.append(c_ast.NamedInitializer(
                [c_ast.ID('offset')],
                self.offsetof(struct.name, prop_name)
            ))
            if nullable:
                exprs.append(c_ast.NamedInitializer(
                    [c_ast.ID('null_offset')],
                    self.offsetof(struct.name, Annotations.NULLABLE_PROPERTY_NAME(prop_name))
                ))
            if arraydecl:
                # static array
                exprs.append(c_ast.NamedInitializer(
                    [c_ast.ID('length')],
                    arraydecl.dim
                ))
            elif types['json'] == 'json_type_array':
                # calculate length offset
                len_prop = Annotations.LENGTH_PROPERTY_NAME(prop_name)
                extra_decls[len_prop] = 'int'
                exprs.append(c_ast.NamedInitializer(
                    [c_ast.ID('length_offset')],
                    self.offsetof(struct.name, len_prop)
                ))
                exprs.append(c_ast.NamedInitializer(
                    [c_ast.ID('dereference')],
                    c_ast.Constant('int', '1')
                ))

            if types['json'] == 'json_type_array':
                # assume PtrDecl for now
                exprs.append(c_ast.NamedInitializer(
                    [c_ast.ID('stride')],
                    self.sizeof(decl.type.type)
                ))


            fi = getframeinfo(currentframe())
            return c_ast.InitList(exprs, Coord(fi.filename, fi.lineno))

        fi = getframeinfo(currentframe())
        extra_decls = {}
        exprs = (make_prop_init_list(p, extra_decls) for p in struct.decls)
        exprs = [e for e in exprs if e is not None]
        # NULL terminator
        exprs.append(c_ast.InitList([c_ast.Constant('int', '0')]))

        initlist = c_ast.InitList(
            exprs,
            Coord(fi.filename, fi.lineno)
        )

        extra_decls = [make_extra_decl(name, t) for name, t in extra_decls.iteritems()]

        return (initlist, extra_decls)

    def offsetof(self, struct_name, name):
        return c_ast.FuncCall(c_ast.ID('offsetof'), c_ast.ExprList([
            c_ast.Typename(None, [],
                c_ast.TypeDecl(None, [],
                    c_ast.Struct(struct_name, None))
            ),
            c_ast.ID(name)
        ]))

    def anonymize_type_decl(self, type_decl):
        types = (c_ast.PtrDecl, c_ast.TypeDecl)
        def anonymize(slot):
            obj = getattr(type_decl, slot)
            if obj.__class__ in types:
                obj = self.anonymize_type_decl(obj)
            if slot == 'declname':
                obj = None
            return obj

        slots = [s for s in type_decl.__class__.__slots__
                 if not s in ('coord', '__weakref__')]
        args = [anonymize(s) for s in slots]
        return type_decl.__class__(*args)

    def sizeof(self, type_decl):
        anonymous_type_decl = self.anonymize_type_decl(type_decl)
        return c_ast.UnaryOp('sizeof', c_ast.Typename(None, [], anonymous_type_decl))

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
            prop_init_list, struct_extra_decls = self.get_property_init_list(struct)
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
                prop_init_list, # init
                None # bitsize
            )
            struct.decls.extend(struct_extra_decls)
            if not ext.seek(n):
                raise ExpansionError(a, n, "Couldn't seek to node")
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
            r'((?:^|(?!\n))#(?P<ppname>define|ifn?def|else|endif|include)' +
            # match preprocessor statement contents including line continuations
            r'(?:[ \t]+(?P<ppcontent>.*?(?:\\\n.*?)*))?(?=\n|$|//|/\*))|' +
            # match oneline comments
            r'(?://[ \t]*@(?P<olname>' + ANNOTATION_NAME + r') *' +
            # oneline annotation content
            r'(?P<olcontent>.*)?)|' +
            # match the entire multiline comment for line counting purposes
            r'/\*(?P<mlwhole>(?:\s)*?@' +
            # match annotation name
            r'(?P<mlname>' + ANNOTATION_NAME + r')\s*' +
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

            wholecontent = match.group('ppcontent') or match.group('mlwhole')
            linecount = wholecontent.count('\n') if wholecontent else 0

            if match.group('nl'):
                line = line + 1
            elif name:
                if ppname:
                    content = match.group('ppcontent')
                else:
                    content = match.group('olcontent') if olname else match.group('mlcontent')
                annotation = {
                    'line': line,
                    'lineEnd': line + linecount,
                    'name': name,
                    'content': content,
                }
                if ppname is not None:
                    annotation['directive'] = ppname
                annotations.append(annotation)
            else:
                break

            line = line + linecount

        self.len = len(annotations)
        self.idx = 0

        self.annotations = annotations
