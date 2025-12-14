#include "morph_runtime.h"
int main() {
    Fox_InitRuntime();
    FoxVal* i = Fox_Int_New(0);
    while (1) {
        if (!Fox_IsTrue(Fox_Lt(i, Fox_Int_New(3)))) break;
        Fox_Print(i);
        FoxVal* t1 = Fox_Add(i, Fox_Int_New(1));
        Fox_DecRef(i);
        i = t1;
    }
    if (Fox_IsTrue(Fox_Eq(i, Fox_Int_New(3)))) {
        Fox_Print(Fox_String_New("Selesai"));
    }
    Fox_ShutdownRuntime();
    return 0;
}
