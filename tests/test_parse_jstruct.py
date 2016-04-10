import unittest2

def td(dir):
    """
    join dir with test directory (wherever this script lives)
    """
    import os
    from inspect import currentframe, getframeinfo
    fi = getframeinfo(currentframe())
    return os.path.join(os.path.dirname(fi.filename), dir)


class TestParseJStruct(unittest2.TestCase):
    def setUp(self):
        with open(td('data/basic.h'), 'r') as infile:
            self.basic_h = infile.read()
        with open(td('data/basic.init.c'), 'r') as infile:
            self.basic_init_c = infile.read()
        with open(td('data/basic.jstruct.h'), 'r') as infile:
            self.basic_jstruct_h = infile.read()

    def test_nest_named_initializer(self):
        from pycparser import c_parser
        from parse.jstruct_generator import CGenerator
        source = '''struct test
            {
                int i;
                struct test_i_t
                {
                    int k;
                } test_i;
                int j;
            };
            struct test test_var = {.i = 0, .test_i = {.k = 1}, .j = 2};
        '''.replace('    ', '')
        ast = c_parser.CParser().parse(source)

        source2 = CGenerator().visit(ast).replace('  ', '')
        self.assertEqual(source2, source)

    def test_top_level_id_hack(self):
        from pycparser import c_parser, c_ast
        from parse.jstruct_generator import CGenerator
        source = 'int i;\n'
        ast = c_parser.CParser().parse(source)
        ast.ext.append(c_ast.ID('#define unobtanium'))
        source2 = CGenerator().visit(ast)
        source = source + '#define unobtanium\n'
        self.assertEqual(source2, source)

    def test_annotations(self):
        from parse.annotations import Annotations
        annotations = Annotations(self.basic_jstruct_h).annotations
        for a in annotations:
            del a['line']
            del a['lineEnd']
        self.maxDiff = 100000
        expected = [
            {'content': 'BASIC_H', 'directive': 'ifndef', 'name': '#'},
            {'content': 'BASIC_H', 'directive': 'define', 'name': '#'},
            {'content': '<stdint.h>', 'directive': 'include', 'name': '#'},
            {'content': '<stdbool.h>', 'directive': 'include', 'name': '#'},
            {'content': '<jstruct/error.h>', 'directive': 'include', 'name': '#'},
            {'content': '', 'name': 'json'},
            {'content': '{\n        "title": "ID",\n        "description": "unique object id",\n        "type": "int"\n    }\n    ', 'name': 'schema'},
            {'content': 'DONT_USE_UINT64_T_FOR_SOME_REASON', 'directive': 'ifndef', 'name': '#'},
            {'content': None, 'directive': 'else', 'name': '#'},
            {'content': None, 'directive': 'endif', 'name': '#'},
            {'content': '', 'name': 'private'},
            {'content': '', 'name': 'nullable'},
            {'content': 'other_name', 'name': 'name'},
            {'content': '', 'name': 'json'},
            {'content': '', 'name': 'json'},
            {'content': '', 'directive': 'endif', 'name': '#'}
        ]

        self.assertEqual(annotations, expected)

    def _normalize_src(self, h):
        """
        Normalize source for differencing
        """
        import re
        # remove whitespace in multiples, and between .;,{} or before/after ={}
        no_ws = re.compile(r'\s{2,}|(?<=[.;,{}])\s+(?=[.;,{}])|(?<=[={}])\s+|\s+(?=[{}=])')
        # add back newlines after {} when followed by ,. or after ;
        return re.sub(r'([{}](?=[,.])|;)', '\\1\n',
            # remove all newlines except when followed by # or /
            re.sub(r'\n(?![#\/])', '',
                # scrub whitespace
                no_ws.sub('', h)
            )
        )

    def test_parser(self):
        import parse
        from parse.jstruct_parse import parse_and_generate
        self.maxDiff = None

        header, init = parse_and_generate(
            td('data/basic.jstruct.h'),
            None, None,
            [td('../'), td('../util/fake_libc_include/')]
        )
        basic_h_stripped = self._normalize_src(self.basic_h)
        basic_init_c_stripped = self._normalize_src(self.basic_init_c)
        header_stripped = self._normalize_src(header)
        init_stripped = self._normalize_src(init)

        self.assertEqual(header_stripped, basic_h_stripped)
        self.assertEqual(init_stripped, basic_init_c_stripped)
