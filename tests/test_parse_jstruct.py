import unittest2

class TestParseJStruct(unittest2.TestCase):
    def setUp(self):
        with open('tests/data/basic.h', 'r') as infile:
            self.basic_h = infile.read()
        with open('tests/data/basic.jstruct.h', 'r') as infile:
            self.basic_jstruct_h = infile.read()

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
            {'content': '', 'line': 31, 'lineEnd': 31, 'name': 'array'},
            {'content': '', 'line': 35, 'lineEnd': 35, 'name': 'json'},
            {'content': '', 'line': 43, 'lineEnd': 43, 'name': 'array'},
            {'content': '', 'line': 47, 'lineEnd': 47, 'directive': 'endif', 'name': '#'}
        ]
        print(repr(annotations.annotations))
        self.assertEqual(annotations.annotations, expected)

    def test_parser(self):
        import parse
        from parse.jstruct_parse import parse_and_generate

        generated = parse_and_generate(
            'tests/data/basic.jstruct.h',
            None,
            ['lib', 'tests/fake_libc_include']
        )
        self.assertEqual(generated, self.basic_h)
