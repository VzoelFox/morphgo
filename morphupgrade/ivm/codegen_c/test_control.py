import sys
import os

current_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.abspath(os.path.join(current_dir, "../../"))
sys.path.append(root_dir)

from transisi.absolute_sntx_morph import Bagian, Tulis, Konstanta, DeklarasiVariabel, Identitas, FoxBinary, Assignment, Selama, JikaMaka
from transisi.morph_t import Token, TipeToken
from ivm.codegen_c.transpiler import CTranspiler

# biar i = 0
token_i = Token(TipeToken.NAMA, "i", 1, 1)
const_0 = Konstanta(Token(TipeToken.ANGKA, 0, 1, 1))
decl_i = DeklarasiVariabel(Token(TipeToken.BIAR, "biar", 1, 1), token_i, const_0)

# Loop Condition: i < 3
const_3 = Konstanta(Token(TipeToken.ANGKA, 3, 1, 1))
op_lt = Token(TipeToken.KURANG_DARI, "<", 1, 1)
cond_loop = FoxBinary(Identitas(token_i), op_lt, const_3)

# Loop Body:
# tulis(i)
print_i = Tulis([Identitas(token_i)])

# ubah i = i + 1
const_1 = Konstanta(Token(TipeToken.ANGKA, 1, 1, 1))
op_plus = Token(TipeToken.TAMBAH, "+", 1, 1)
expr_inc = FoxBinary(Identitas(token_i), op_plus, const_1)
assign_inc = Assignment(Identitas(token_i), expr_inc)

body_loop = Bagian([print_i, assign_inc])
loop_node = Selama(Token(TipeToken.SELAMA, "selama", 1, 1), cond_loop, body_loop)

# After loop:
# jika i == 3 maka tulis("Selesai")
const_str_done = Konstanta(Token(TipeToken.TEKS, "Selesai", 1, 1))
op_eq = Token(TipeToken.SAMA_DENGAN, "==", 1, 1)
cond_if = FoxBinary(Identitas(token_i), op_eq, const_3)
body_if = Bagian([Tulis([const_str_done])])
if_node = JikaMaka(cond_if, body_if, [], None)

ast_root = Bagian([decl_i, loop_node, if_node])

# Transpile
transpiler = CTranspiler()
c_code = transpiler.compile(ast_root)

print(c_code)
