from pycparser import c_generator, c_ast

class CGenerator(c_generator.CGenerator):
    def visit_FileAST(self, n):
        """
        Slightly different than pycparser's version. Top level ids are #directives, no ;
        """
        s = ''
        for ext in n.ext:
            if isinstance(ext, c_ast.FuncDef):
                s += self.visit(ext)
            elif isinstance(ext, c_ast.ID):
                s += self.visit(ext) + '\n'
            else:
                s += self.visit(ext) + ';\n'
        return s


    def visit_NamedInitializer(self, n):
        from pycparser import c_ast
        s = ''
        for name in n.name:
            if isinstance(name, c_ast.ID):
                s += '.' + name.name
            elif isinstance(name, c_ast.Constant):
                s += '[' + name.value + ']'
        s += ' = ' + self._visit_expr(n.expr)
        return s
