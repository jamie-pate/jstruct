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
        annotations = Annotations(self.basic_jstruct_h)
        expected = [
            {'content': 'BASIC_H', 'line': 1, 'lineEnd': 1, 'directive': 'ifndef', 'name': '#'},
            {'content': 'BASIC_H', 'line': 2, 'lineEnd': 2, 'directive': 'define', 'name': '#'},
            {'content': '<stdint.h>', 'line': 4, 'lineEnd': 4, 'directive': 'include', 'name': '#'},
            {'content': '<stdbool.h>', 'line': 5, 'lineEnd': 5, 'directive': 'include', 'name': '#'},
            {'content': '', 'line': 7, 'lineEnd': 7, 'name': 'json'},
            {'content': '{\n        "title": "ID",\n        "description": "unique object id",\n        "type": "int"\n    }\n    ', 'line': 9, 'lineEnd': 15, 'name': 'schema'},
            {'content': '', 'line': 19, 'lineEnd': 19, 'name': 'private'},
            {'content': '', 'line': 25, 'lineEnd': 25, 'name': 'nullable'},
            {'content': 'other_name', 'line': 27, 'lineEnd': 27, 'name': 'name'},
            {'content': '', 'line': 33, 'lineEnd': 33, 'name': 'json'},
            {'content': '', 'line': 41, 'lineEnd': 41, 'name': 'array'},
            {'content': '', 'line': 45, 'lineEnd': 45, 'directive': 'endif', 'name': '#'}
        ]

        self.assertEqual(annotations.annotations, expected)

    def _normalize_src(self, h):
        """
        Normalize source for differencing
        """
        import re
        # remove whitespace in multiples, and between .;,{} or before/after ={}
        no_ws = re.compile(r'\s{2,}|(?<=[.;,{}])\s+(?=[.;,{}])|(?<=[={}])\s+|\s+(?=[{}=])')
        # add back newlines after {} when followed by ,. or after ;
        return re.sub(r'([{}](?=[,.])|;)', '\\1\n',
            # remove all newlines
            re.sub(r'\n', '',
                # scrub whitespace
                no_ws.sub('', h)
            )
        )

    def test_parser(self):
        import parse
        from parse.jstruct_parse import parse_and_generate
        self.maxDiff = None

        generated = parse_and_generate(
            td('data/basic.jstruct.h'),
            None,
            [td('../lib'), td('fake_libc_include')]
        )
        basic_h_stripped = self._normalize_src(self.basic_h)
        gen_stripped = self._normalize_src(generated)

        self.assertEqual(gen_stripped, basic_h_stripped)
