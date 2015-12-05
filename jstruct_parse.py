#-----------------------------------------------------------------
# libjstruct: jstruct_parse.py
#
# https://github.com/jamie-pate/jstruct
#
# Copyright (C) 2015, Jamie Pate
# License: MIT
#-----------------------------------------------------------------

from __future__ import print_function
import sys

from pycparser import parse_file, c_parser, c_generator, c_ast
import re

class Annotations():
    def __init__(self, filename):
        self.annotations = self._get_annotations(filename)
        self.len = len(self.annotations)
        self.idx = 0

    def get(self, line):
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

    def _get_annotations(self, filename):
        source = ''
        with open(filename, "r") as infile:
            source = infile.read()
        ANNOTATION_NAME = r'[a-zA-Z_]+'

        # lex + yacc and i'm using a regex? HERESY!!
        annotation_expr = re.compile(
            r'(?:' +
            # match newlines (so we can count them)
            r'(?P<nl>\n)|' +
            # match oneline comments
            r'//[\n\s]*@(?P<olname>' + ANNOTATION_NAME + r')|' +
            # oneline annotation content
            r'(?P<olcontent>.*)' +
            # match the entire multiline comment for line counting purposes
            r'/\*(?P<mlwhole>(?:[\n\s])*?@' +
            # match annotation name
            r'(?P<mlname>' + ANNOTATION_NAME + r')[\s\n]*' +
            # match everything after the @annotation in the comment
            r'(?P<mlcontent>(?:\n|.)*?)'+
            # end of multiline comment and non-capturing group
            r')\*/)',)
        annotations = []
        line = 1
        match = True
        pos = 0

        while match:

            match = annotation_expr.search(source, pos)
            if not match:
                break

            pos = match.end()
            olname = match.group('olname')
            mlname = match.group('mlname')
            name = olname or mlname
            linecount = match.group('mlwhole').count('\n') if mlname else 0

            if match.group('nl'):
                line = line + 1
            elif name:
                content = match.group('olcontent') if olname else match.group('mlcontent')
                annotations.append({
                    'line': line,
                    'lineEnd': line + linecount,
                    'name': name,
                    'content': content,
                })
            else:
                break

            line = line + linecount

        return annotations


def parse_jstruct(filename):
    parser = c_parser.CParser()
    ast = parse_file(filename, use_cpp=True);
    generator = c_generator.CGenerator()

    return (ast, generator)


def main(filename):

    annotations = Annotations(filename)

    ast, generator = parse_jstruct(filename)

    # print(generator.visit(ast))

    # Uncomment the following line to see the AST in a nice, human
    # readable way. show() is the most useful tool in exploring ASTs
    # created by pycparser. See the c_ast.py file for the options you
    # can pass it.
    #
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


main('tests/data/basic.jstruct.h')
