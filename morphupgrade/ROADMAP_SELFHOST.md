# Roadmap Self-Host Morph (Transpiler C)

Status: **Phase 3 Complete (Partial Phase 4)**
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
- [x] Aritmatika (`+`, `-`, `*`, `/`).
- [x] Kontrol Alur Dasar (`jika`, `selama`).

### Fase 3: Struktur Data & Fungsi (Selesai)
- [x] Tipe List (`[]`) via `FoxList`.
- [x] Deklarasi Fungsi (`fungsi`).
- [x] Panggilan Fungsi (Mapping ke Fungsi C Native).
- [ ] Tipe Map (`{}`).
- [ ] Closure (Lingkup Variabel).

### Fase 4: Sistem Objek (Next)
- [ ] Kelas & Instance.
- [ ] Pewarisan.
- [ ] `ini` context.

### Fase 5: Self-Compilation (The Goal)
- [ ] Transpile `greenfield/kompiler/*.fox` menggunakan `ivm`.
- [ ] Hasilnya (`compiler.c`) dicompile dengan GCC.
- [ ] Binary `compiler_native` dihasilkan.
- [ ] `ivm` (Python) dipensiunkan.
