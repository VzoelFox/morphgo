#include "morph_runtime.h"

FoxVal* Fox_Nil;
FoxVal* Fox_True;
FoxVal* Fox_False;

static FoxVal* _alloc_val(size_t size, FoxType type) {
    FoxVal* v = (FoxVal*)malloc(size);
    v->type = type;
    v->ref_count = 1;
    return v;
}

void Fox_InitRuntime() {
    Fox_Nil = _alloc_val(sizeof(FoxVal), FOX_NIL);

    Fox_True = _alloc_val(sizeof(FoxBool), FOX_BOOL);
    ((FoxBool*)Fox_True)->value = true;

    Fox_False = _alloc_val(sizeof(FoxBool), FOX_BOOL);
    ((FoxBool*)Fox_False)->value = false;
}

void Fox_ShutdownRuntime() {
    // Leaking globals for now
}

FoxVal* Fox_Int_New(long long val) {
    FoxInt* v = (FoxInt*)_alloc_val(sizeof(FoxInt), FOX_INT);
    v->value = val;
    return (FoxVal*)v;
}

FoxVal* Fox_String_New(const char* val) {
    FoxString* v = (FoxString*)_alloc_val(sizeof(FoxString), FOX_STRING);
    v->length = strlen(val);
    v->data = malloc(v->length + 1);
    strcpy(v->data, val);
    return (FoxVal*)v;
}

void Fox_Print(FoxVal* val) {
    if (!val) { printf("NULL"); return; }
    switch(val->type) {
        case FOX_NIL: printf("nil"); break;
        case FOX_INT: printf("%lld", ((FoxInt*)val)->value); break;
        case FOX_BOOL: printf(((FoxBool*)val)->value ? "benar" : "salah"); break;
        case FOX_STRING: printf("%s", ((FoxString*)val)->data); break;
        default: printf("<objek %d>", val->type);
    }
    printf("\n");
}

void Fox_IncRef(FoxVal* val) {
    if (val) val->ref_count++;
}

void Fox_DecRef(FoxVal* val) {
    if (!val) return;
    val->ref_count--;
    if (val->ref_count <= 0) {
        if (val->type == FOX_STRING) {
            free(((FoxString*)val)->data);
        }
        if (val != Fox_Nil && val != Fox_True && val != Fox_False) {
            free(val);
        }
    }
}

bool Fox_IsTrue(FoxVal* val) {
    if (!val) return false;
    if (val == Fox_False || val == Fox_Nil) return false;
    if (val->type == FOX_INT) return ((FoxInt*)val)->value != 0;
    return true;
}

// --- Arithmetic ---
FoxVal* Fox_Add(FoxVal* a, FoxVal* b) {
    if (a->type == FOX_INT && b->type == FOX_INT) {
        return Fox_Int_New(((FoxInt*)a)->value + ((FoxInt*)b)->value);
    }
    // TODO: String concat
    return Fox_Nil;
}

FoxVal* Fox_Sub(FoxVal* a, FoxVal* b) {
    if (a->type == FOX_INT && b->type == FOX_INT) {
        return Fox_Int_New(((FoxInt*)a)->value - ((FoxInt*)b)->value);
    }
    return Fox_Nil;
}

FoxVal* Fox_Mul(FoxVal* a, FoxVal* b) {
    if (a->type == FOX_INT && b->type == FOX_INT) {
        return Fox_Int_New(((FoxInt*)a)->value * ((FoxInt*)b)->value);
    }
    return Fox_Nil;
}

FoxVal* Fox_Div(FoxVal* a, FoxVal* b) {
    if (a->type == FOX_INT && b->type == FOX_INT) {
        long long bv = ((FoxInt*)b)->value;
        if (bv == 0) return Fox_Nil; // TODO: Error
        return Fox_Int_New(((FoxInt*)a)->value / bv);
    }
    return Fox_Nil;
}

// --- Comparison ---
FoxVal* Fox_Eq(FoxVal* a, FoxVal* b) {
    if (a == b) return Fox_True;
    if (a->type != b->type) return Fox_False;
    if (a->type == FOX_INT) {
        return ((FoxInt*)a)->value == ((FoxInt*)b)->value ? Fox_True : Fox_False;
    }
    return Fox_False;
}

FoxVal* Fox_Lt(FoxVal* a, FoxVal* b) {
    if (a->type == FOX_INT && b->type == FOX_INT) {
        return ((FoxInt*)a)->value < ((FoxInt*)b)->value ? Fox_True : Fox_False;
    }
    return Fox_False;
}

FoxVal* Fox_Gt(FoxVal* a, FoxVal* b) {
    if (a->type == FOX_INT && b->type == FOX_INT) {
        return ((FoxInt*)a)->value > ((FoxInt*)b)->value ? Fox_True : Fox_False;
    }
    return Fox_False;
}
