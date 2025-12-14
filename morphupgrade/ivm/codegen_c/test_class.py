import sys
import os

current_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.abspath(os.path.join(current_dir, "../../"))
sys.path.append(root_dir)

from transisi.absolute_sntx_morph import Bagian, Tulis, Konstanta, DeklarasiVariabel, Identitas, FoxBinary, Assignment, Kelas, FungsiDeklarasi, PanggilFungsi, AmbilProperti, PernyataanEkspresi
from transisi.morph_t import Token, TipeToken
from ivm.codegen_c.transpiler import CTranspiler

# class Manusia
token_Manusia = Token(TipeToken.NAMA, "Manusia", 1, 1)

# func sapa(nama)
token_sapa = Token(TipeToken.NAMA, "sapa", 1, 1)
token_nama = Token(TipeToken.NAMA, "nama", 1, 1)

# body: tulis("Halo " + nama)
const_Halo = Konstanta(Token(TipeToken.TEKS, "Halo ", 1, 1))
op_plus = Token(TipeToken.TAMBAH, "+", 1, 1)
expr_add = FoxBinary(const_Halo, op_plus, Identitas(token_nama))
stmt_print = Tulis([expr_add])
body_sapa = Bagian([stmt_print])

func_sapa = FungsiDeklarasi(token_sapa, [token_nama], body_sapa)

# Class Node
class_Manusia = Kelas(token_Manusia, None, [func_sapa])

# biar orang = Manusia()
call_ctor = PanggilFungsi(Identitas(token_Manusia), Token(TipeToken.KURUNG_TUTUP, ")", 1, 1), [])
token_orang = Token(TipeToken.NAMA, "orang", 1, 1)
decl_orang = DeklarasiVariabel(Token(TipeToken.BIAR, "biar", 1, 1), token_orang, call_ctor)

# orang.sapa("Budi")
access_sapa = AmbilProperti(Identitas(token_orang), token_sapa)
const_Budi = Konstanta(Token(TipeToken.TEKS, "Budi", 1, 1))
call_sapa = PanggilFungsi(access_sapa, Token(TipeToken.KURUNG_TUTUP, ")", 1, 1), [const_Budi])
stmt_call = PernyataanEkspresi(call_sapa)

ast_root = Bagian([class_Manusia, decl_orang, stmt_call])

# Transpile
transpiler = CTranspiler()
c_code = transpiler.compile(ast_root)

print(c_code)
