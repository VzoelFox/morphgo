# Roadmap Self-Host Morph (Transpiler C)

Status: **Phase 1 & 2 Complete (In Progress)**
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

### Fase 3: Struktur Data & Fungsi (Next)
- [ ] Tipe List (`[]`) dan Map (`{}`).
- [ ] Deklarasi Fungsi (`fungsi`).
- [ ] Panggilan Fungsi (Stack Management).
- [ ] Closure (Lingkup Variabel).

### Fase 4: Sistem Objek (Hard)
- [ ] Kelas & Instance.
- [ ] Pewarisan.
- [ ] `ini` context.

### Fase 5: Self-Compilation (The Goal)
- [ ] Transpile `greenfield/kompiler/*.fox` menggunakan `ivm`.
- [ ] Hasilnya (`compiler.c`) dicompile dengan GCC.
- [ ] Binary `compiler_native` dihasilkan.
- [ ] `ivm` (Python) dipensiunkan.
