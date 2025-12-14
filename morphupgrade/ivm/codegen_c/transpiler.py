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

    def _compile_function_body(self, node, raw_name, class_name=None):
        params = [p.nilai for p in node.parameter]

        self.headers.append(f"FoxVal* {raw_name}(int argc, FoxVal** argv);")

        prev_buffer = self.current_buffer
        self.current_buffer = self.functions
        self.indent_level = 0

        self.write(f"FoxVal* {raw_name}(int argc, FoxVal** argv) {{")
        self.indent_level += 1

        old_locals = self.locals.copy()
        self.locals = set()

        arg_idx = 0
        if class_name:
            self.locals.add("ini")
            self.write(f"FoxVal* ini = (argc > {arg_idx}) ? argv[{arg_idx}] : Fox_Nil;")
            arg_idx += 1

        for p_name in params:
            self.locals.add(p_name)
            self.write(f"FoxVal* {p_name} = (argc > {arg_idx}) ? argv[{arg_idx}] : Fox_Nil;")
            arg_idx += 1

        self.visit(node.badan)

        self.write("return Fox_Nil;")
        self.indent_level -= 1
        self.write("}")
        self.write("")

        self.current_buffer = prev_buffer
        self.locals = old_locals

    def visit_FungsiDeklarasi(self, node):
        raw_name = f"impl_{node.nama.nilai}"
        self._compile_function_body(node, raw_name)

        var_name = node.nama.nilai
        self.locals.add(var_name)
        self.write(f'FoxVal* {var_name} = Fox_Function_New({raw_name}, "{var_name}");')

    def visit_Kelas(self, node):
        class_name = node.nama.nilai
        self.locals.add(class_name)
        self.write(f'FoxVal* {class_name} = Fox_Class_New("{class_name}");')

        for method in node.metode:
            method_name = method.nama.nilai
            raw_method_name = f"impl_{class_name}_{method_name}"
            self._compile_function_body(method, raw_method_name, class_name=class_name)
            self.write(f'Fox_Class_AddMethod({class_name}, "{method_name}", Fox_Function_New({raw_method_name}, "{method_name}"));')

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
        if node.target.__class__.__name__ == 'Akses':
             obj = self.visit_expression(node.target.objek)
             key = self.visit_expression(node.target.kunci)
             val = self.visit_expression(node.nilai)
             self.write(f'Fox_SetItem({obj}, {key}, {val});')
             return

        if node.target.__class__.__name__ == 'AmbilProperti':
             obj = self.visit_expression(node.target.objek)
             prop_name = node.target.nama.nilai
             val = self.visit_expression(node.nilai)
             self.write(f'Fox_SetAttr({obj}, "{prop_name}", {val});')
             return

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

    def visit_Jodohkan(self, node):
        expr_val = self.visit_expression(node.ekspresi)
        val_var = self.get_temp_var()
        self.write(f'FoxVal* {val_var} = {expr_val};')

        first = True
        for kasus in node.kasus:
             pola = kasus.pola
             cond = self._compile_pattern_check(pola, val_var)

             if kasus.jaga:
                 guard_expr = self.visit_expression(kasus.jaga)
                 cond = f"({cond}) && Fox_IsTrue({guard_expr})"

             prefix = "if" if first else "else if"
             self.write(f'{prefix} ({cond}) {{')
             self.indent_level += 1

             self._compile_pattern_bind(pola, val_var)
             self.visit(kasus.badan)

             self.indent_level -= 1
             self.write('}')
             first = False

    def _compile_pattern_check(self, pola, val_var):
        name = pola.__class__.__name__
        if name == 'PolaLiteral':
             lit_val = self.visit_expression(pola.nilai)
             return f"Fox_IsTrue(Fox_Eq({val_var}, {lit_val}))"
        elif name == 'PolaWildcard':
             return "true"
        elif name == 'PolaIkatanVariabel':
             return "true"
        elif name == 'PolaVarian':
             class_name = pola.nama.nilai
             return f"Fox_IsInstance({val_var}, {class_name})"
        return "false"

    def _compile_pattern_bind(self, pola, val_var):
        name = pola.__class__.__name__
        if name == 'PolaIkatanVariabel':
             var_name = pola.token.nilai
             self.locals.add(var_name)
             self.write(f'FoxVal* {var_name} = {val_var};')
             self.write(f'Fox_IncRef({var_name});')

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
        callee = self.visit_expression(node.callee)
        count = len(node.argumen)

        args_var = self.get_temp_var()
        if count > 0:
            self.write(f"FoxVal* {args_var}[{count}];")
            for i, arg in enumerate(node.argumen):
                val = self.visit_expression(arg)
                self.write(f"{args_var}[{i}] = {val};")
            return f"Fox_Call({callee}, {count}, {args_var})"
        else:
            return f"Fox_Call({callee}, 0, NULL)"

    def visit_expr_Konstanta(self, node):
        val = node.nilai
        if isinstance(val, int):
            return f"Fox_Int_New({val})"
        elif isinstance(val, str):
            return f'Fox_String_New("{val}")'
        return "Fox_Nil"

    def visit_expr_Identitas(self, node):
        return node.nama

    def visit_expr_Ini(self, node):
        return "ini"

    def visit_expr_Daftar(self, node):
        count = len(node.elemen)
        temp_list = self.get_temp_var()
        self.write(f'FoxVal* {temp_list} = Fox_List_New({count});')
        for elem in node.elemen:
            val = self.visit_expression(elem)
            self.write(f'Fox_List_Append({temp_list}, {val});')
        return temp_list

    def visit_expr_Kamus(self, node):
        temp_dict = self.get_temp_var()
        self.write(f'FoxVal* {temp_dict} = Fox_Dict_New();')
        for k, v in node.pasangan:
            key_expr = self.visit_expression(k)
            val_expr = self.visit_expression(v)
            self.write(f'Fox_Dict_Set({temp_dict}, {key_expr}, {val_expr});')
        return temp_dict

    def visit_expr_Akses(self, node):
        obj = self.visit_expression(node.objek)
        key = self.visit_expression(node.kunci)
        return f"Fox_GetItem({obj}, {key})"

    def visit_expr_AmbilProperti(self, node):
        obj = self.visit_expression(node.objek)
        prop_name = node.nama.nilai
        return f"Fox_GetAttr({obj}, \"{prop_name}\")"

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
