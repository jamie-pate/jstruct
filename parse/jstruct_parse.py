from annotations import Annotations
from preprocess import preprocess
from pycparser import c_parser, c_generator, c_ast
import re


GENERATED = '// Generated automatically by libjstruct. Do Not Modify.\n\n'
PREPEND_HEADERS = '#include <jstruct.h>\n#include <json-c/json_object.h>\n\n'

def parse_jstruct(filename, include_paths=[]):
    parser = c_parser.CParser()
    with open(filename, 'r') as infile:
        text = infile.read()

    # insert some header includes and a 'do not modify'
    text = re.sub(
        r'^[\s\n]*#ifndef\s+[A-Z_]+[\s\n]+#define\s+[A-Z_]+\s*\n',
        r'\g<0>' + GENERATED + PREPEND_HEADERS,
        text, count=1, flags=re.IGNORECASE
    )

    pptext, err = preprocess(text,
        include_paths=include_paths,
        defines=['__attribute__(x)=']
    )
    if err:
        raise Exception('C Preprocessor error:' + err)

    ast = parser.parse(pptext, filename=filename)
    generator = c_generator.CGenerator()

    return (ast, generator, text)


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
    ast.ext = [e for e in ast.ext if e.coord is None or e.coord.file == filename]


def parse_and_generate(filename, outfilename=None, include_paths=[]):
    from os import path

    ast, generator, text = parse_jstruct(filename, include_paths=include_paths)
    annotations = Annotations(text)

    annotations.expand(ast, '<stdin>')
    prune_ast(ast, '<stdin>')

    result = generator.visit(ast)

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
