import sys
import os

current_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.abspath(os.path.join(current_dir, "../../"))
sys.path.append(root_dir)

from transisi.absolute_sntx_morph import Bagian, Tulis, Konstanta, DeklarasiVariabel, Identitas, Kamus, Akses, Assignment
from transisi.morph_t import Token, TipeToken
from ivm.codegen_c.transpiler import CTranspiler

# biar d = {"nama": "Jules", "umur": 2}
token_d = Token(TipeToken.NAMA, "d", 1, 1)

pair1 = (Konstanta(Token(TipeToken.TEKS, "nama", 1, 1)), Konstanta(Token(TipeToken.TEKS, "Jules", 1, 1)))
pair2 = (Konstanta(Token(TipeToken.TEKS, "umur", 1, 1)), Konstanta(Token(TipeToken.ANGKA, 2, 1, 1)))
dict_expr = Kamus([pair1, pair2])

decl_d = DeklarasiVariabel(Token(TipeToken.BIAR, "biar", 1, 1), token_d, dict_expr)

# tulis(d["nama"])
access_nama = Akses(Identitas(token_d), Konstanta(Token(TipeToken.TEKS, "nama", 1, 1)))
print_nama = Tulis([access_nama])

# ubah d["umur"] = 3
access_umur = Akses(Identitas(token_d), Konstanta(Token(TipeToken.TEKS, "umur", 1, 1)))
const_3 = Konstanta(Token(TipeToken.ANGKA, 3, 1, 1))
assign_umur = Assignment(access_umur, const_3)

# tulis(d["umur"])
print_umur = Tulis([access_umur])

# tulis(d)
print_d = Tulis([Identitas(token_d)])

ast_root = Bagian([decl_d, print_nama, assign_umur, print_umur, print_d])

# Transpile
transpiler = CTranspiler()
c_code = transpiler.compile(ast_root)

print(c_code)
