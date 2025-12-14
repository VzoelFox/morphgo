from transisi.morph_t import TipeToken

class CTranspiler:
    def __init__(self):
        self.code_buffer = []
        self.indent_level = 0
        self.locals = set()
        self.temp_var_counter = 0

    def compile(self, node):
        self.write_header()
        self.visit(node)
        self.write_footer()
        return "\n".join(self.code_buffer)

    def write(self, text):
        self.code_buffer.append("    " * self.indent_level + text)

    def write_header(self):
        self.write('#include "morph_runtime.h"')
        self.write('int main() {')
        self.indent_level += 1
        self.write('Fox_InitRuntime();')

    def write_footer(self):
        self.write('Fox_ShutdownRuntime();')
        self.write('return 0;')
        self.indent_level -= 1
        self.write('}')

    def get_temp_var(self):
        self.temp_var_counter += 1
        return f"t{self.temp_var_counter}"

    def visit(self, node):
        method_name = f'visit_{node.__class__.__name__}'
        visitor = getattr(self, method_name, self.generic_visit)
        return visitor(node)

    def generic_visit(self, node):
        raise NotImplementedError(f"C Transpiler belum mendukung node: {node.__class__.__name__}")

    def visit_Bagian(self, node):
        for stmt in node.daftar_pernyataan:
            self.visit(stmt)

    def visit_Tulis(self, node):
        for arg in node.argumen:
            val_expr = self.visit_expression(arg)
            self.write(f'Fox_Print({val_expr});')

    def visit_DeklarasiVariabel(self, node):
        var_name = node.nama.nilai

        if var_name in self.locals:
             if node.nilai:
                 val_expr = self.visit_expression(node.nilai)
                 temp = self.get_temp_var()
                 self.write(f'FoxVal* {temp} = {val_expr};')
                 self.write(f'Fox_DecRef({var_name});')
                 self.write(f'{var_name} = {temp};')
        else:
             self.locals.add(var_name)
             if node.nilai:
                 val_expr = self.visit_expression(node.nilai)
                 self.write(f'FoxVal* {var_name} = {val_expr};')
             else:
                 self.write(f'FoxVal* {var_name} = Fox_Nil;')

    def visit_Assignment(self, node):
        var_name = node.target.nama
        val_expr = self.visit_expression(node.nilai)
        temp = self.get_temp_var()
        self.write(f'FoxVal* {temp} = {val_expr};')
        self.write(f'Fox_DecRef({var_name});')
        self.write(f'{var_name} = {temp};')

    def visit_JikaMaka(self, node):
        cond_expr = self.visit_expression(node.kondisi)
        self.write(f'if (Fox_IsTrue({cond_expr})) {{')
        self.indent_level += 1
        self.visit(node.blok_maka)
        self.indent_level -= 1
        self.write('}')
        if node.blok_lain:
            self.write('else {')
            self.indent_level += 1
            self.visit(node.blok_lain)
            self.indent_level -= 1
            self.write('}')

    def visit_Selama(self, node):
        self.write(f'while (1) {{')
        self.indent_level += 1
        cond = self.visit_expression(node.kondisi)
        self.write(f'if (!Fox_IsTrue({cond})) break;')
        self.visit(node.badan)
        self.indent_level -= 1
        self.write('}')

    def visit_expression(self, node):
        method_name = f'visit_expr_{node.__class__.__name__}'
        visitor = getattr(self, method_name, self.generic_visit_expr)
        return visitor(node)

    def generic_visit_expr(self, node):
         raise NotImplementedError(f"Expression not supported: {node.__class__.__name__}")

    def visit_expr_Konstanta(self, node):
        val = node.nilai
        if isinstance(val, int):
            return f"Fox_Int_New({val})"
        elif isinstance(val, str):
            return f'Fox_String_New("{val}")'
        return "Fox_Nil"

    def visit_expr_Identitas(self, node):
        return node.nama

    def visit_expr_FoxBinary(self, node):
        left = self.visit_expression(node.kiri)
        right = self.visit_expression(node.kanan)
        op = node.op.tipe

        if op == TipeToken.TAMBAH: return f"Fox_Add({left}, {right})"
        if op == TipeToken.KURANG: return f"Fox_Sub({left}, {right})"
        if op == TipeToken.KALI: return f"Fox_Mul({left}, {right})"
        if op == TipeToken.BAGI: return f"Fox_Div({left}, {right})"
        if op == TipeToken.SAMA_DENGAN: return f"Fox_Eq({left}, {right})"
        if op == TipeToken.KURANG_DARI: return f"Fox_Lt({left}, {right})"
        if op == TipeToken.LEBIH_DARI: return f"Fox_Gt({left}, {right})"

        raise NotImplementedError(f"Operator {node.op.nilai} belum didukung di Transpiler C")
