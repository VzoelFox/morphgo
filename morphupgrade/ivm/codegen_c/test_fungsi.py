import sys
import os

current_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.abspath(os.path.join(current_dir, "../../"))
sys.path.append(root_dir)

from transisi.absolute_sntx_morph import Bagian, Tulis, Konstanta, DeklarasiVariabel, Identitas, FoxBinary, Assignment, FungsiDeklarasi, PernyataanKembalikan, PanggilFungsi, PernyataanEkspresi
from transisi.morph_t import Token, TipeToken
from ivm.codegen_c.transpiler import CTranspiler

# fungsi tambah(a, b)
token_tambah = Token(TipeToken.NAMA, "tambah", 1, 1)
param_a = Token(TipeToken.NAMA, "a", 1, 1)
param_b = Token(TipeToken.NAMA, "b", 1, 1)

# body: kembali a + b
op_plus = Token(TipeToken.TAMBAH, "+", 1, 1)
expr_add = FoxBinary(Identitas(param_a), op_plus, Identitas(param_b))
ret_stmt = PernyataanKembalikan(Token(TipeToken.KEMBALI, "kembali", 1, 1), expr_add)
body_func = Bagian([ret_stmt])

func_decl = FungsiDeklarasi(token_tambah, [param_a, param_b], body_func)

# Call: tambah(10, 20)
const_10 = Konstanta(Token(TipeToken.ANGKA, 10, 1, 1))
const_20 = Konstanta(Token(TipeToken.ANGKA, 20, 1, 1))
call_expr = PanggilFungsi(Identitas(token_tambah), Token(TipeToken.KURUNG_TUTUP, ")", 1, 1), [const_10, const_20])

# biar hasil = call
token_hasil = Token(TipeToken.NAMA, "hasil", 1, 1)
decl_hasil = DeklarasiVariabel(Token(TipeToken.BIAR, "biar", 1, 1), token_hasil, call_expr)

# tulis(hasil)
print_hasil = Tulis([Identitas(token_hasil)])

ast_root = Bagian([func_decl, decl_hasil, print_hasil])

# Transpile
transpiler = CTranspiler()
c_code = transpiler.compile(ast_root)

print(c_code)
