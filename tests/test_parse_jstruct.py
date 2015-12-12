import unittest2

class TestParseJStruct(unittest2.TestCase):
    def test_parser(self):
        import parse
        from parse.jstruct_parse import parse_and_generate

        generated = parse_and_generate(
            'tests/data/basic.jstruct.h',
            None,
            ['lib', 'tests/fake_libc_include']
        )
        with open('tests/data/basic.h', 'r') as infile:
            basic_h = infile.read()
        self.assertEqual(generated, basic_h)
