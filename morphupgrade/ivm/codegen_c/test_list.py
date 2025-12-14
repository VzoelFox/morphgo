import sys
import os

current_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.abspath(os.path.join(current_dir, "../../"))
sys.path.append(root_dir)

from transisi.absolute_sntx_morph import Bagian, Tulis, Konstanta, DeklarasiVariabel, Identitas, Daftar
from transisi.morph_t import Token, TipeToken
from ivm.codegen_c.transpiler import CTranspiler

# biar l = [1, 2]
token_l = Token(TipeToken.NAMA, "l", 1, 1)
const_1 = Konstanta(Token(TipeToken.ANGKA, 1, 1, 1))
const_2 = Konstanta(Token(TipeToken.ANGKA, 2, 1, 1))
list_expr = Daftar([const_1, const_2])

decl_l = DeklarasiVariabel(Token(TipeToken.BIAR, "biar", 1, 1), token_l, list_expr)

# tulis(l)
print_l = Tulis([Identitas(token_l)])

ast_root = Bagian([decl_l, print_l])

# Transpile
transpiler = CTranspiler()
c_code = transpiler.compile(ast_root)

print(c_code)
