# Roadmap Self-Host Morph (Transpiler C)

Status: **Phase 5.1 Complete (Pattern Matching Implemented)**
Target: **Native Binary Compiler**

## Architecture Baru
*   **Compiler Frontend:** Morph (saat ini dijalankan via `ivm` Python).
*   **Compiler Backend:** C Code Generator (Output `.c`).
*   **Native Compiler:** GCC (untuk mengubah `.c` menjadi executable).
*   **Runtime:** `morph_runtime.c` (Micro Runtime C, Garbage Collection via RefCount).

## Fase Pengerjaan

### Fase 1: Scaffolding (Selesai)
- [x] Struktur Direktori `codegen_c` & `transpiler_c`.
- [x] `morph_runtime.h` (Definisi Tipe `FoxVal`).
- [x] Integrasi `ivm` (Python) untuk memanggil Transpiler.

### Fase 2: Fitur Dasar (Selesai)
- [x] Output Teks (`tulis`).
- [x] Variabel (`biar`, `ubah`).
- [x] Aritmatika (`+`, `-`, `*`, `/`) termasuk String Concat.
- [x] Kontrol Alur Dasar (`jika`, `selama`).

### Fase 3: Struktur Data & Fungsi (Selesai)
- [x] Tipe List (`[]`) via `FoxList`.
- [x] Tipe Map (`{}`) via `FoxDict` (Hash Map).
- [x] Deklarasi Fungsi (`fungsi`).
- [x] Panggilan Fungsi (Generic `Fox_Call`).
- [ ] Closure (Lingkup Variabel - Masih Global/Local sederhana).

### Fase 4: Sistem Objek (Selesai)
- [x] Kelas & Instance (`FoxClass`, `FoxInstance`).
- [x] Pewarisan (Belum diimplementasikan).
- [x] `ini` context (Bound Method).
- [x] Akses Properti (`.`).

### Fase 5: Self-Compilation (The Goal)
- [x] Implementasi `Jodohkan` (Pattern Matching - `PolaVarian`, `PolaLiteral`).
- [ ] Implementasi `CobaTangkap` (Exception Handling).
- [ ] Transpile `greenfield/kompiler/*.fox` menggunakan `ivm`.
- [ ] Hasilnya (`compiler.c`) dicompile dengan GCC.
- [ ] Binary `compiler_native` dihasilkan.
- [ ] `ivm` (Python) dipensiunkan.
