#include "morph_runtime.h"
FoxVal* impl_tambah(int argc, FoxVal** argv);

FoxVal* impl_tambah(int argc, FoxVal** argv) {
    FoxVal* a = (argc > 0) ? argv[0] : Fox_Nil;
    FoxVal* b = (argc > 1) ? argv[1] : Fox_Nil;
    return Fox_Add(a, b);
    return Fox_Nil;
}


int main() {
    Fox_InitRuntime();
    FoxVal* tambah = Fox_Function_New(impl_tambah, "tambah");
    FoxVal* t1[2];
    t1[0] = Fox_Int_New(10);
    t1[1] = Fox_Int_New(20);
    FoxVal* hasil = Fox_Call(tambah, 2, t1);
    Fox_Print(hasil);
    Fox_ShutdownRuntime();
    return 0;
}
