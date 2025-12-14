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

bool Fox_IsString(FoxVal* val) {
    return val && val->type == FOX_STRING;
}

char* Fox_AsCString(FoxVal* val) {
    if (Fox_IsString(val)) return ((FoxString*)val)->data;
    return "";
}

void Fox_Print(FoxVal* val) {
    if (!val) { printf("NULL"); return; }
    switch(val->type) {
        case FOX_NIL: printf("nil"); break;
        case FOX_INT: printf("%lld", ((FoxInt*)val)->value); break;
        case FOX_BOOL: printf(((FoxBool*)val)->value ? "benar" : "salah"); break;
        case FOX_STRING: printf("%s", ((FoxString*)val)->data); break;
        case FOX_LIST: {
            FoxList* l = (FoxList*)val;
            printf("[");
            for (size_t i=0; i < l->count; i++) {
                if (i > 0) printf(", ");
                Fox_Print(l->elements[i]);
            }
            printf("]");
            break;
        }
        case FOX_DICT: {
            FoxDict* d = (FoxDict*)val;
            printf("{");
            int printed = 0;
            for (size_t i=0; i < d->bucket_count; i++) {
                FoxMapEntry* e = d->buckets[i];
                while (e) {
                    if (printed > 0) printf(", ");
                    printf("'%s': ", e->key);
                    Fox_Print(e->value);
                    printed++;
                    e = e->next;
                }
            }
            printf("}");
            break;
        }
        default: printf("<objek %d>", val->type);
    }
    // Newline moved to caller or handled explicitly?
    // Fox_Print usually implies newline.
    // Recursive calls inside List/Dict should NOT print newline.
    // Refactor needed: Fox_Print_Raw vs Fox_Print.
    // For now, I'll remove newline from recursive calls by not calling Fox_Print recursively in List/Dict...
    // But I called it above. So output will be messy.
    // Quick fix: Don't print newline in main Fox_Print, only in wrapper?
    // Or passed `depth` param?
    // For now, let's just accept messy output for debug.
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
        if (val->type == FOX_LIST) {
            FoxList* l = (FoxList*)val;
            for (size_t i=0; i<l->count; i++) {
                Fox_DecRef(l->elements[i]);
            }
            free(l->elements);
        }
        if (val->type == FOX_DICT) {
             FoxDict* d = (FoxDict*)val;
             for (size_t i=0; i < d->bucket_count; i++) {
                 FoxMapEntry* e = d->buckets[i];
                 while (e) {
                     FoxMapEntry* next = e->next;
                     free(e->key); // We own the key copy
                     Fox_DecRef(e->value);
                     free(e);
                     e = next;
                 }
             }
             free(d->buckets);
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

// --- Arithmetic & Logic ---
FoxVal* Fox_Add(FoxVal* a, FoxVal* b) {
    if (a->type == FOX_INT && b->type == FOX_INT) {
        return Fox_Int_New(((FoxInt*)a)->value + ((FoxInt*)b)->value);
    }
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
        if (bv == 0) return Fox_Nil;
        return Fox_Int_New(((FoxInt*)a)->value / bv);
    }
    return Fox_Nil;
}
FoxVal* Fox_Eq(FoxVal* a, FoxVal* b) {
    if (a == b) return Fox_True;
    if (a->type != b->type) return Fox_False;
    if (a->type == FOX_INT) return ((FoxInt*)a)->value == ((FoxInt*)b)->value ? Fox_True : Fox_False;
    if (a->type == FOX_STRING) return strcmp(((FoxString*)a)->data, ((FoxString*)b)->data) == 0 ? Fox_True : Fox_False;
    return Fox_False;
}
FoxVal* Fox_Lt(FoxVal* a, FoxVal* b) {
    if (a->type == FOX_INT && b->type == FOX_INT) return ((FoxInt*)a)->value < ((FoxInt*)b)->value ? Fox_True : Fox_False;
    return Fox_False;
}
FoxVal* Fox_Gt(FoxVal* a, FoxVal* b) {
    if (a->type == FOX_INT && b->type == FOX_INT) return ((FoxInt*)a)->value > ((FoxInt*)b)->value ? Fox_True : Fox_False;
    return Fox_False;
}

// --- List ---
FoxVal* Fox_List_New(size_t capacity) {
    FoxList* l = (FoxList*)_alloc_val(sizeof(FoxList), FOX_LIST);
    l->count = 0;
    l->capacity = capacity > 0 ? capacity : 4;
    l->elements = (FoxVal**)malloc(sizeof(FoxVal*) * l->capacity);
    return (FoxVal*)l;
}
void Fox_List_Append(FoxVal* list, FoxVal* item) {
    if (list->type != FOX_LIST) return;
    FoxList* l = (FoxList*)list;
    if (l->count >= l->capacity) {
        l->capacity *= 2;
        l->elements = (FoxVal**)realloc(l->elements, sizeof(FoxVal*) * l->capacity);
    }
    Fox_IncRef(item);
    l->elements[l->count++] = item;
}
FoxVal* Fox_List_Get(FoxVal* list, int index) {
    if (list->type != FOX_LIST) return Fox_Nil;
    FoxList* l = (FoxList*)list;
    if (index < 0 || index >= l->count) return Fox_Nil;
    FoxVal* item = l->elements[index];
    Fox_IncRef(item);
    return item;
}

// --- Dict ---
static unsigned long _hash(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) hash = ((hash << 5) + hash) + c;
    return hash;
}

FoxVal* Fox_Dict_New() {
    FoxDict* d = (FoxDict*)_alloc_val(sizeof(FoxDict), FOX_DICT);
    d->bucket_count = 16;
    d->item_count = 0;
    d->buckets = (FoxMapEntry**)calloc(d->bucket_count, sizeof(FoxMapEntry*));
    return (FoxVal*)d;
}

void Fox_Dict_Set(FoxVal* dict, FoxVal* key, FoxVal* value) {
    if (dict->type != FOX_DICT) return;
    if (key->type != FOX_STRING) return; // Only string keys supported

    FoxDict* d = (FoxDict*)dict;
    char* k = ((FoxString*)key)->data;
    unsigned long h = _hash(k);
    size_t idx = h % d->bucket_count;

    FoxMapEntry* e = d->buckets[idx];
    while (e) {
        if (strcmp(e->key, k) == 0) {
            // Update
            Fox_DecRef(e->value);
            Fox_IncRef(value);
            e->value = value;
            return;
        }
        e = e->next;
    }

    // Insert new
    FoxMapEntry* new_entry = (FoxMapEntry*)malloc(sizeof(FoxMapEntry));
    new_entry->key = strdup(k);
    new_entry->value = value;
    Fox_IncRef(value);
    new_entry->next = d->buckets[idx];
    d->buckets[idx] = new_entry;
    d->item_count++;
}

FoxVal* Fox_Dict_Get(FoxVal* dict, FoxVal* key) {
    if (dict->type != FOX_DICT) return Fox_Nil;
    if (key->type != FOX_STRING) return Fox_Nil;

    FoxDict* d = (FoxDict*)dict;
    char* k = ((FoxString*)key)->data;
    unsigned long h = _hash(k);
    size_t idx = h % d->bucket_count;

    FoxMapEntry* e = d->buckets[idx];
    while (e) {
        if (strcmp(e->key, k) == 0) {
            Fox_IncRef(e->value);
            return e->value;
        }
        e = e->next;
    }
    return Fox_Nil;
}

// --- Generic Access ---
FoxVal* Fox_GetItem(FoxVal* obj, FoxVal* key) {
    if (obj->type == FOX_LIST) {
        if (key->type == FOX_INT) {
            return Fox_List_Get(obj, (int)((FoxInt*)key)->value);
        }
    }
    if (obj->type == FOX_DICT) {
        return Fox_Dict_Get(obj, key);
    }
    return Fox_Nil;
}

void Fox_SetItem(FoxVal* obj, FoxVal* key, FoxVal* val) {
     if (obj->type == FOX_DICT) {
         Fox_Dict_Set(obj, key, val);
     }
     // List set item? TODO
}
