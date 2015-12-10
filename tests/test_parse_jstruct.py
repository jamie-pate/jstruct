import unittest2

class TestParseJStruct(unittest2.TestCase):
    def test_parser(self):
        import parse
        from parse.jstruct_parse import parse_and_generate

        generated = parse_and_generate('tests/data/basic.jstruct.h', None)
        with open('tests/data/basic.h') as infile:
            basic_h = infile.read()
        self.assertEqual(generated, basic_h)
