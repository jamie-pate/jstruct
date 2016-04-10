#!/usr/bin/env python

from annotations import Annotations
from jstruct_generator import CGenerator
from preprocess import preprocess
from pycparser import c_parser, c_ast
import re


GENERATED1NL = '// Generated automatically by libjstruct. Do Not Modify.\n'
GENERATED = GENERATED1NL + '\n'
INIT_INSTRUCTIONS = '// This file must be included directly in a single c file.\n\n'
INCLUDE_H = '#include "{0}"\n'.format
# these prepended headers get parsed, then output.
# TODO: figure out how to add a comment after?
PREPEND_HEADERS = '#include {JSTRUCT_H}\n#include {JSON_OBJECT_H}\n'.format
GUARD_HEADERS_EXPR = re.compile(
    r'^\s*#ifndef\s+[A-Z_]+\s+#define\s+[A-Z_]+\s*\n',
    flags=re.IGNORECASE
)
# TODO: allow more than 2 dots?
FILENAME_EXPR = re.compile(
    r'^(?P<basename>[^.]+)\.(?:(?P<ext2>[^.]+)\.)?(?P<ext>[^.]+)$'
)

def parse_jstruct(filename, include_paths=[], defines=[]):
    parser = c_parser.CParser()
    with open(filename, 'r') as infile:
        text = infile.read()
    define_map = {
        'JSTRUCT_H': '<jstruct/jstruct.h>',
        'JSON_OBJECT_H': '<json-c/json_object.h>',
        'ARRAYLIST_H': '<json-c/arraylist.h>'
    }
    define_map.update({ds[0]: ds[1] for ds in (d.split('=') for d in defines)})
    defines = ['{0}={1}'.format(*kv) for kv in define_map.iteritems()]

    # insert some header includes and a 'do not modify'
    text = re.sub(
        GUARD_HEADERS_EXPR,
        r'\g<0>' + GENERATED + PREPEND_HEADERS(**define_map),
        text, count=1
    )
    pptext, err = preprocess(text,
        include_paths=include_paths,
        defines=['__attribute__(x)='] + defines
    )
    if err:
        import os
        rel_filename = os.path.relpath(filename)
        err = err.replace('<stdin>', rel_filename)
        print(repr(defines))
        raise Exception('C Preprocessor: ' + err)

    ast = parser.parse(pptext, filename=filename)

    return (ast, text)


def prune_ast(ast, filename):
    """
    prune the ast which will contain all the definitions of all the included headers tool
    """
    ast.ext = [e for e in ast.ext if e.coord is None or e.coord.file == filename]

def split_ast(ast):
    """
    split the ast into header and initializer headers.
    initializers cannot be included in more than one translation unit (c file)
    """
    init_decls = []

    def extern_inits(decl):
        # c_ast.ID is used for directives. pass them straight through to both files
        if isinstance(decl, c_ast.ID):
            init_decls.append(decl)
            return decl
        elif decl.init:
            init_decls.append(c_ast.Decl(
                decl.name,
                decl.quals,
                decl.storage,
                decl.funcspec,
                decl.type,
                decl.init,
                decl.bitsize
            ))
            decl.storage = ['extern']
            decl.init = None
            return decl
        else:
            return decl

    out_ast = c_ast.FileAST([extern_inits(e) for e in ast.ext])
    init_ast = c_ast.FileAST(init_decls)
    return (out_ast, init_ast)

def parse_and_generate(filename, out_filename=None, init_filename=None, include_paths=[], defines=[]):
    """
    parse the file at filename.
    if out_filename and init_filename are None:return a tuple containing the generated file's names.
    otherwise return the generated source code for each
    """
    from os import path

    if out_filename:
        out_filename = re.sub(FILENAME_EXPR, out_filename, filename)
    if init_filename:
        init_filename = re.sub(FILENAME_EXPR, init_filename, filename)
    rel_filename = ''
    if out_filename is None and init_filename is None:
        rel_filename = re.sub(FILENAME_EXPR, r'\g<basename>.h', path.basename(filename))
    else:
        init_dir = path.dirname(init_filename)
        rel_filename = path.relpath(out_filename, init_dir)

    ast, text = parse_jstruct(filename, include_paths=include_paths, defines=defines)
    annotations = Annotations(text)

    annotations.expand(ast, '<stdin>')
    prune_ast(ast, '<stdin>')
    out_ast, init_ast = split_ast(ast)

    generator = CGenerator()
    out_result = generator.visit(out_ast)
    init_result = generator.visit(init_ast)
    if GUARD_HEADERS_EXPR.search(out_result):
        out_result = re.sub(
            GUARD_HEADERS_EXPR,
            r'\g<0>' + GENERATED,
            out_result, count=1
        ) + '\n#endif\n'
    else:
        out_result = GENERATED + out_result
    init_result = re.sub(GUARD_HEADERS_EXPR, '', init_result)
    init_instructions = INIT_INSTRUCTIONS if init_filename and init_filename.endswith('.h') else ''
    init_result = GENERATED1NL + init_instructions + INCLUDE_H(rel_filename) + init_result

    if out_filename:
        with open(out_filename, 'w') as out_file:
            out_file.write(out_result)
    if init_filename:
        with open(init_filename, 'w') as init_file:
            init_file.write(init_result)

    if out_filename is None and init_filename is None:
        return (out_result, init_result)
    else:
        return (out_filename, init_filename)

if __name__ == '__main__':
    import argparse
    argparser = argparse.ArgumentParser(
        description='Parse x.jstruct.h file and generate x.h and x.init.h ')

    argparser.add_argument('infile',
        metavar='jstruct_header_filename',
        type=str,
        help='x.jstruct.h file to be parsed'
    )
    argparser.add_argument('-o', '--out', dest='outfile', type=str,
        default=r'\g<basename>.h',
        help='output header file name. (python re.sub() repl syntax)')
    argparser.add_argument('-i', '--init', dest='initfile', type=str,
        default=r'\g<basename>.init.c',
        help='initializer code file name. (python re.sub() repl syntax)')
    argparser.add_argument('-s', '--silent', action='store_true',
        help='silent mode')
    argparser.add_argument('-D', '--define', action='append', default=[])
    argparser.add_argument('includedir', type=str, nargs='*',
        help='override/extra directories to pass to the c preprocessor with -I. ' +
            '(Suggested: util/fake_libc_include)'
    )
    args = argparser.parse_args()
    outfile, initfile = parse_and_generate(
        args.infile,
        args.outfile,
        args.initfile,
        args.includedir,
        args.define
    )
    if not args.silent:
        remember = '\nRemember to include {1} directly in a single .c file' \
            if args.initfile.endswith('.h') else ''
        print(
            ('Success: {0} and {1} generated successfully.' +
             remember)
            .format(outfile, initfile)
        )
