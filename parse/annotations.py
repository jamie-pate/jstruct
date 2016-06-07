import sys
import StringIO
import re
import json
from inspect import currentframe, getframeinfo
from itertools import chain
from pycparser import c_ast
from pycparser.plyparser import Coord


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
        self.filename = None
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

        if self.filename:
            node_show = node_show.replace('<stdin>', self.filename)

        try:
            return 'Unable to process annotation @{0} at line {1}{2}\n{3}\n{4}'.format(
                self.annotation.name if self.annotation else None,
                self.annotation.line if self.annotation else None,
                message,
                'Annotation: ' + repr(self.annotation),
                'Node: ' + node_show
            )
        except Exception as ex:
            return str(ex)


class Annotation():
    def __init__(self, name, content, line=None, linecount=None, directive=None, root=None):
        self.name = name
        self.content = content
        self.line = line if line is not None else root.line
        self.line_end = self.line + linecount if linecount is not None else root.line_end

        if directive is None and root is not None:
            self.directive = root.directive
        else:
            self.directive = directive

    def __str__(self):
        return repr(self.dict())

    def __repr__(self):
        return str(self)

    def dict(self): 
        result = {
            'name': self.name,
            'content': self.content
        }
        if self.line is not None:
            result['line'] = self.line
        if self.line_end is not None:
            result['line_end'] = self.line_end
        if self.directive is not None:
            result['directive'] = self.directive
        return result


class AnnotatedProperty():
    """
    Contains:
    init_list: list of `NamedInitializer`s for a struct which describes this property.
    decl: decl associated with this property.
    extra_decls: dictionary of 'property': 'c_type' decls to append to support this property
    nullable: True if nullable
    """

    LENGTH_NAME = '{0}__length__'.format
    NULLABLE_NAME = '{0}__null__'.format
    def __init__(self, annotated_struct, decl):
        self.init_list = None
        self.decl = decl
        self.extra_decls = {}
        self.values = {}
        init_exprs = []

        self.struct = annotated_struct

        annotations = self.struct.annotations.get(decl, self.struct.json_annotations)
        try:
            for a in annotations:
                expr = self.expand_annotation(a, self.values)
                if expr is not None:
                    init_exprs.append(expr)
        except StopIteration:
            name = a.name
            a = next(annotations, None)
            if a is not None:
                raise ExpansionError(a, decl, 'Unexpected annotation after ' + name)
            return
        init_list = []

        prop_name = type_find(decl, c_ast.TypeDecl).declname

        # name the property if it hasn't already been named
        if 'name' not in self.values:
            self.values['name'] = prop_name

        # add string initializers
        for init_name in ('name', 'schema'):
            if init_name in self.values:
                literal = str_literal(self.values[init_name])
                init_exprs.append(
                    c_ast.NamedInitializer(
                        [c_ast.ID(init_name)],
                        c_ast.Constant('string', literal)
                    )
                )

        # assign types
        type_annotations = {
            name: t for name,t in self.values.iteritems()
            if isinstance(t, Annotation)
        }
        types, arraydecl = self.struct.annotations.get_types(decl, type_annotations)
        type_inits = [c_ast.NamedInitializer(
            [c_ast.ID(ttype)],
            c_ast.ID(t)
        ) for ttype, t in types.iteritems()]
        fi = getframeinfo(currentframe())
        init_exprs.append(c_ast.NamedInitializer(
            [c_ast.ID('type')],
            c_ast.InitList(type_inits, Coord(fi.filename, fi.lineno))
        ))
        struct = annotated_struct.struct
        # calculate struct offset
        init_exprs.append(c_ast.NamedInitializer(
            [c_ast.ID('offset')],
            self.offsetof(struct.name, prop_name)
        ))
        if 'nullable' in self.values and self.values['nullable']:
            self.extra_decls[AnnotatedProperty.NULLABLE_NAME(decl.name)] = 'bool'
            init_exprs.append(c_ast.NamedInitializer(
                [c_ast.ID('null_offset')],
                self.offsetof(struct.name, AnnotatedProperty.NULLABLE_NAME(prop_name))
            ))
        if arraydecl:
            # static array
            init_exprs.append(c_ast.NamedInitializer(
                [c_ast.ID('length')],
                arraydecl.dim
            ))
        elif types['json'] == 'json_type_array':
            # calculate length offset
            len_prop = AnnotatedProperty.LENGTH_NAME(prop_name)
            self.extra_decls[len_prop] = 'int'
            init_exprs.append(c_ast.NamedInitializer(
                [c_ast.ID('length_offset')],
                self.offsetof(struct.name, len_prop)
            ))
            init_exprs.append(c_ast.NamedInitializer(
                [c_ast.ID('dereference')],
                c_ast.Constant('int', '1')
            ))

        if types['json'] == 'json_type_array':
            # assume PtrDecl for now
            init_exprs.append(c_ast.NamedInitializer(
                [c_ast.ID('stride')],
                self.sizeof(decl.type.type)
            ))


        fi = getframeinfo(currentframe())
        self.init_list = c_ast.InitList(init_exprs, Coord(fi.filename, fi.lineno))


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
        import inspect

        # NOTE: __slots__ doesn't exist in ubuntu's version (2.10+dfsg-3)
        # of pycparser. best to use the github version. (>= 2.11)
        slots = [s for s in type_decl.__class__.__slots__
                 if not s in ('coord', '__weakref__')]
        args = [anonymize(s) for s in slots]
        return type_decl.__class__(*args)

    def sizeof(self, type_decl):
        anonymous_type_decl = self.anonymize_type_decl(type_decl)
        return c_ast.UnaryOp('sizeof', c_ast.Typename(None, [], anonymous_type_decl))

    def expand_annotation(self, a, values):
        """ 
        Expand an annotation by adding a key/value to values and/or returning
        an Expr or None.
        Adding the annotation to values will cause it to be a 'type annotation'
        which overrides the type initializer for the json_type_x and/or
        jstruct_extra_type_x
        Raises StopIteration if there can be no more valid annotations, and the
        AnnotatedProperty should be skipped from the json property list.
        """
        name = a.name
        if name == '#':
            return None
        # skip private properties
        if name in ('private', 'inline'):
            values[name] = a
            raise StopIteration()

        # append the contents of these annotations directly later.
        if name in ['schema', 'name']:
            values[name] = a.content
            if a.content == None:
                raise ExpansionError(a, decl, 'Content is None')
            return None
            
        if name == 'nullable':
            init_name = c_ast.ID(name)
            expr = c_ast.Constant('int', '1')
            values['nullable'] = True
            return c_ast.NamedInitializer([init_name], expr)

        if name in ['array']:
            values[name] = a
            return None

        raise ExpansionError(a, decl, 'Unexpected annotation')



class AnnotatedStruct():
    """
    Describes the property list for a struct.
    annotations: Annotations object
    annotated_properties: list of annotated properties
    init_list: initlist to generate metadata
    decls: a list of all `Decl` objects to use in the new struct definition.
    struct: the Struct that's being annotated
    """
    def __init__(self, annotations, struct, json_annotations, ext):
        """
        Describes the property list for a struct
        Also create a list of c_ast.Decl to append to the struct decls
        """
        self.json_annotations = json_annotations
        self.annotated_properties = None
        self.annotations = annotations
        self.ext = ext
        self.init_list = None
        self.decls = None
        self.struct = struct
        self.extra_decls = None
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

        

        fi = getframeinfo(currentframe())
        annotated_properties = [AnnotatedProperty(self, d) for d in struct.decls]

        out_ap = []
        for ap in annotated_properties:
            inline_annotation = ap.values.get('inline', False)
            if inline_annotation:
                astruct = self.inline_struct_annotated(inline_annotation, ap.decl)
                out_ap += astruct.annotated_properties
            else:
                out_ap.append(ap)
        
        self.annotated_properties = out_ap

        init_lists = [
            ap.init_list for ap in out_ap
            # 'private' and 'inline' have no init_list
            if ap.init_list is not None
        ]

        # NULL terminator
        init_lists.append(c_ast.InitList([c_ast.Constant('int', '0')]))

        self.init_list = c_ast.InitList(
            init_lists,
            Coord(fi.filename, fi.lineno)
        )

        decls = [ap.decl for ap in out_ap]

        extra_decls = chain.from_iterable((
            ap.extra_decls.iteritems()
            for ap in out_ap
        ))
        extra_decls = [make_extra_decl(name, t) for name, t in extra_decls]

        decls += extra_decls

        self.decls = decls

    def inline_struct_annotated(self, a, decl):
        """
        Import all the decls from decl's type, replacing decl in the current AnnotatedStruct
        """
        name = decl.type.type.name
        try:
            struct = next((
                e.type for e in self.ext
                if hasattr(e, 'type') and hasattr(e.type, 'name') and e.type.name == name
            ))
        except Exception as ex:
            raise ExpansionError(a, decl, ex.message)

        struct = c_ast.Struct(self.struct.name, struct.decls, struct.coord)
        return AnnotatedStruct(self.annotations, struct, self.json_annotations, self.ext)


class Annotations():

    # TODO: make these configurable?
    PROPERTIES_NAME = '{0}__jstruct_properties__'.format
    JSON_TYPE_NAME = 'json_type_{0}'.format

    @staticmethod
    def JSTRUCT_EXTRA_TYPE_NAME(t):
        return 'jstruct_extra_type_{0}'.format(t.replace(' ', '_'))

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

    def get(self, decl, json_annotations=None):
        """
        Return a generator of all
        annotations for the specified line.
        line must be larger than any previous call,
        except after calling parse() or reset()
        """
        if json_annotations:
            root_annotation = json_annotations['@root']

            if decl.name in json_annotations:
                annotation = json_annotations[decl.name]
                if isinstance(annotation, dict):
                    for name, a in annotation.iteritems():
                        if name.startswith('@'):
                            yield Annotation(name[1:], a, root=root_annotation)
                elif annotation.startswith('@'):
                    yield Annotation(annotation[1:], '', root=root_annotation)

        line = decl.coord.line
        a = True
        while a:
            a = self.get_next(line)
            if a:
                yield a


    def get_next(self, line):
        if self.idx >= self.len:
            return None

        if self.annotations[self.idx].line <= line:
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
            'char*': 'string',
        }
        # types found here will have a jstruct_extra_type
        # assigned as well as the json_type
        # TODO: generate these instead?
        extra_type_map = {
            'uint32_t': 'int',
            'int64_t': 'int',
            'uint64_t': 'int',
            'unsigned int': 'int',
            'long long': 'int',
            'unsigned long long': 'int',
        }
        # should probably just use this instead of raising errors!
        all_type_map = extra_type_map.copy()
        all_type_map.update(json_type_map)

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
                                ('\nUnable to map [{0}] to json type\n' +
                                 'Available types are: {1}')
                                .format(ctype, all_type_map)
                            )
                        result['extra'] = Annotations.JSTRUCT_EXTRA_TYPE_NAME(ctypename)

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
        initial_tb = None
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
                        initial_tb = sys.exc_info()[2]

        raise Exception, initial_err, initial_tb

    def expand(self, ast, filename):
        """
        Expand a pycparser ast with extra structures/data etc
        """
        idx = 0

        self._extract_ast_info(ast)

        struct_object_property_decl = c_ast.Struct('jstruct_object_property', None)

        def ppdirective(a, n, ext):
            id = ' '.join((a.directive, a.content)) if a.content else a.directive
            ext.insert(c_ast.ID('#{0}'.format(id)))
            return False

        def annotate_struct(a, n, ext):

            if not isinstance(n.type, c_ast.Struct):
                raise ExpansionError(a, n,
                    'Cannot expand annotation @{0} on {1}'
                    .format(a.name, n.__class__.__name__))

            json_annotations = {}
            if a.name == 'json' and a.content != '':
                json_annotations = json.loads(a.content)
                json_annotations['@root'] = a
                if not isinstance(json_annotations, dict):
                    raise ExpansionError(a, n,
                        'Expected @json content to be empty or a JSON object.\nGot {0}'
                        .format(a.content)) 
            name = n.type.name
            struct = n.type
            annotated_struct = AnnotatedStruct(self, struct, json_annotations, ext)
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
                annotated_struct.init_list, # init
                None # bitsize
            )

            struct.decls = annotated_struct.decls
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

            annotations = self.get(n)
            for a in annotations:
                if not done and a.name in process:
                    done = process[a.name](a, n, ext)
                    if done:
                        try:
                            a_name = a.name
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
                if content:
                    content = content.strip()
                annotation = Annotation(name, content, line, linecount, ppname)
                annotations.append(annotation)
            else:
                break

            line = line + linecount

        self.len = len(annotations)
        self.idx = 0

        self.annotations = annotations
