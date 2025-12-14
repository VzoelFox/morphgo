from transisi.morph_t import TipeToken

class CTranspiler:
    def __init__(self):
        self.headers = []
        self.functions = []
        self.main_body = []
        self.current_buffer = self.main_body
        self.indent_level = 0
        self.locals = set()
        self.temp_var_counter = 0

    def compile(self, node):
        self.write_header()
        self.visit(node)
        return self.assemble_code()

    def assemble_code(self):
        code = []
        code.append('#include "morph_runtime.h"')
        code.extend(self.headers)
        code.append("")
        code.extend(self.functions)
        code.append("")
        code.append("int main() {")
        code.append("    Fox_InitRuntime();")
        for line in self.main_body:
            code.append("    " + line)
        code.append("    Fox_ShutdownRuntime();")
        code.append("    return 0;")
        code.append("}")
        return "\n".join(code)

    def write(self, text):
        indent = "    " * self.indent_level
        self.current_buffer.append(indent + text)

    def write_header(self): pass
    def write_footer(self): pass

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
            if stmt.__class__.__name__ == 'FungsiDeklarasi':
                self.visit(stmt)
            else:
                self.visit(stmt)

    def visit_FungsiDeklarasi(self, node):
        func_name = f"fox_user_{node.nama.nilai}"
        params = [f"FoxVal* {p.nilai}" for p in node.parameter]
        param_str = ", ".join(params)

        self.headers.append(f"FoxVal* {func_name}({param_str});")

        prev_buffer = self.current_buffer
        self.current_buffer = self.functions
        self.indent_level = 0

        self.write(f"FoxVal* {func_name}({param_str}) {{")
        self.indent_level += 1

        old_locals = self.locals.copy()
        self.locals = set()
        for p in node.parameter:
            self.locals.add(p.nilai)

        self.visit(node.badan)

        self.write("return Fox_Nil;")

        self.indent_level -= 1
        self.write("}")
        self.write("")

        self.current_buffer = prev_buffer
        self.locals = old_locals

    def visit_PernyataanKembalikan(self, node):
        if node.nilai:
            val = self.visit_expression(node.nilai)
            self.write(f"return {val};")
        else:
            self.write("return Fox_Nil;")

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

    def visit_PernyataanEkspresi(self, node):
        expr = self.visit_expression(node.ekspresi)
        self.write(f"{expr};")

    def visit_expression(self, node):
        method_name = f'visit_expr_{node.__class__.__name__}'
        visitor = getattr(self, method_name, self.generic_visit_expr)
        return visitor(node)

    def generic_visit_expr(self, node):
         if node.__class__.__name__ == 'PanggilFungsi':
             return self.visit_PanggilFungsi(node)
         raise NotImplementedError(f"Expression not supported: {node.__class__.__name__}")

    def visit_PanggilFungsi(self, node):
        if node.callee.__class__.__name__ == 'Identitas':
            func_name = f"fox_user_{node.callee.nama}"
            args = [self.visit_expression(a) for a in node.argumen]
            arg_str = ", ".join(args)
            return f"{func_name}({arg_str})"
        else:
             raise NotImplementedError("Dynamic function call not supported yet")

    def visit_expr_Konstanta(self, node):
        val = node.nilai
        if isinstance(val, int):
            return f"Fox_Int_New({val})"
        elif isinstance(val, str):
            return f'Fox_String_New("{val}")'
        return "Fox_Nil"

    def visit_expr_Identitas(self, node):
        return node.nama

    def visit_expr_Daftar(self, node):
        count = len(node.elemen)
        temp_list = self.get_temp_var()
        self.write(f'FoxVal* {temp_list} = Fox_List_New({count});')
        for elem in node.elemen:
            val = self.visit_expression(elem)
            self.write(f'Fox_List_Append({temp_list}, {val});')
        return temp_list

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
