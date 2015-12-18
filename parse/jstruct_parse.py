from annotations import Annotations
from jstruct_generator import CGenerator
from preprocess import preprocess
from pycparser import c_parser, c_ast
import re


GENERATED = '// Generated automatically by libjstruct. Do Not Modify.\n\n'
# these prepended headers get parsed, then output.
# TODO: figure out how to add a comment after?
PREPEND_HEADERS = '#include <jstruct.h>\n#include <json-c/json_object.h>\n'
# TODO: custom prefix?
LOCAL_JSTRUCT_TYPES = '#include "generated_jstruct_types.h"\n'
GUARD_HEADERS_EXPR = re.compile(
    r'^\s*#ifndef\s+[A-Z_]+\s+#define\s+[A-Z_]+\s*\n',
    flags=re.IGNORECASE
)

def parse_jstruct(filename, include_paths=[]):
    parser = c_parser.CParser()
    with open(filename, 'r') as infile:
        text = infile.read()

    # insert some header includes and a 'do not modify'
    text = re.sub(
        GUARD_HEADERS_EXPR,
        r'\g<0>' + GENERATED + PREPEND_HEADERS,
        text, count=1
    )

    pptext, err = preprocess(text,
        include_paths=include_paths,
        defines=['__attribute__(x)=']
    )
    if err:
        raise Exception('C Preprocessor error:' + err)

    ast = parser.parse(pptext, filename=filename)

    return (ast, text)


def prune_ast(ast, filename):
    """
    prune the ast which will contain all the definitions of all the included headers tool
    """
    ast.ext = [e for e in ast.ext if e.coord is None or e.coord.file == filename]


def parse_and_generate(filename, outfilename=None, include_paths=[]):
    from os import path

    ast, text = parse_jstruct(filename, include_paths=include_paths)
    annotations = Annotations(text)

    annotations.expand(ast, '<stdin>')
    prune_ast(ast, '<stdin>')

    generator = CGenerator()
    result = generator.visit(ast)
    if GUARD_HEADERS_EXPR.search(result):
        result = re.sub(
            GUARD_HEADERS_EXPR,
            r'\g<0>' + GENERATED + LOCAL_JSTRUCT_TYPES,
            result, count=1
        ) + '\n#endif\n'
    else:
        result = GENERATED + LOCAL_JSTRUCT_TYPES

    if outfilename:
        with open(outfilename, 'w') as outfile:
            outfile.write(result)

    return result
