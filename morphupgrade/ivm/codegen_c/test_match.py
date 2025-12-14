import sys
import os

current_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.abspath(os.path.join(current_dir, "../../"))
sys.path.append(root_dir)

from transisi.absolute_sntx_morph import Bagian, Tulis, Konstanta, DeklarasiVariabel, Identitas, Kelas, PanggilFungsi, Jodohkan, JodohkanKasus, PolaVarian, PolaWildcard
from transisi.morph_t import Token, TipeToken
from ivm.codegen_c.transpiler import CTranspiler

# class Angka
token_Angka = Token(TipeToken.NAMA, "Angka", 1, 1)
class_Angka = Kelas(token_Angka, None, [])

# biar x = Angka()
call_ctor = PanggilFungsi(Identitas(token_Angka), Token(TipeToken.KURUNG_TUTUP, ")", 1, 1), [])
token_x = Token(TipeToken.NAMA, "x", 1, 1)
decl_x = DeklarasiVariabel(Token(TipeToken.BIAR, "biar", 1, 1), token_x, call_ctor)

# jodohkan x
# | Angka maka tulis("Ini Angka")
const_str_match = Konstanta(Token(TipeToken.TEKS, "Ini Angka", 1, 1))
body_match = Bagian([Tulis([const_str_match])])
pola_match = PolaVarian(token_Angka, [])
kasus_match = JodohkanKasus(pola_match, None, body_match)

# | _ maka tulis("Bukan")
const_str_else = Konstanta(Token(TipeToken.TEKS, "Bukan", 1, 1))
body_else = Bagian([Tulis([const_str_else])])
pola_wild = PolaWildcard(Token(TipeToken.GARIS_PEMISAH, "_", 1, 1)) # Wildcard usually just needs a token
kasus_wild = JodohkanKasus(pola_wild, None, body_else)

match_node = Jodohkan(Identitas(token_x), [kasus_match, kasus_wild])

ast_root = Bagian([class_Angka, decl_x, match_node])

# Transpile
transpiler = CTranspiler()
c_code = transpiler.compile(ast_root)

print(c_code)
