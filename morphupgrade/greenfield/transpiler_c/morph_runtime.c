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
        case FOX_FUNCTION: printf("<fungsi %s>", ((FoxFunction*)val)->name); break;
        case FOX_CLASS: printf("<kelas %s>", ((FoxClass*)val)->name); break;
        case FOX_INSTANCE: printf("<instance %s>", ((FoxInstance*)val)->klass->name); break;
        case FOX_BOUND_METHOD: printf("<metode terikat %s>", ((FoxBoundMethod*)val)->func->name); break;
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
                     free(e->key);
                     Fox_DecRef(e->value);
                     free(e);
                     e = next;
                 }
             }
             free(d->buckets);
        }
        if (val->type == FOX_FUNCTION) {
            free(((FoxFunction*)val)->name);
        }
        if (val->type == FOX_CLASS) {
            free(((FoxClass*)val)->name);
            Fox_DecRef(((FoxClass*)val)->methods);
        }
        if (val->type == FOX_INSTANCE) {
            Fox_DecRef((FoxVal*)((FoxInstance*)val)->klass);
            Fox_DecRef(((FoxInstance*)val)->fields);
        }
        if (val->type == FOX_BOUND_METHOD) {
            Fox_DecRef((FoxVal*)((FoxBoundMethod*)val)->instance);
            Fox_DecRef((FoxVal*)((FoxBoundMethod*)val)->func);
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
    if (a->type == FOX_STRING && b->type == FOX_STRING) {
        FoxString* sa = (FoxString*)a;
        FoxString* sb = (FoxString*)b;
        size_t len = sa->length + sb->length;
        char* new_str = malloc(len + 1);
        strcpy(new_str, sa->data);
        strcat(new_str, sb->data);
        FoxVal* res = (FoxVal*)Fox_String_New(new_str);
        free(new_str);
        return res;
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
    if (key->type != FOX_STRING) return;

    FoxDict* d = (FoxDict*)dict;
    char* k = ((FoxString*)key)->data;
    unsigned long h = _hash(k);
    size_t idx = h % d->bucket_count;

    FoxMapEntry* e = d->buckets[idx];
    while (e) {
        if (strcmp(e->key, k) == 0) {
            Fox_DecRef(e->value);
            Fox_IncRef(value);
            e->value = value;
            return;
        }
        e = e->next;
    }

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

// --- Object API ---

FoxVal* Fox_Function_New(FoxCFunc func, const char* name) {
    FoxFunction* f = (FoxFunction*)_alloc_val(sizeof(FoxFunction), FOX_FUNCTION);
    f->func_ptr = func;
    f->name = strdup(name);
    return (FoxVal*)f;
}

FoxVal* Fox_Class_New(const char* name) {
    FoxClass* c = (FoxClass*)_alloc_val(sizeof(FoxClass), FOX_CLASS);
    c->name = strdup(name);
    c->methods = Fox_Dict_New();
    return (FoxVal*)c;
}

void Fox_Class_AddMethod(FoxVal* klass, const char* name, FoxVal* func) {
    if (klass->type != FOX_CLASS) return;
    FoxVal* key = Fox_String_New(name);
    Fox_Dict_Set(((FoxClass*)klass)->methods, key, func);
    Fox_DecRef(key);
}

FoxVal* Fox_Instance_New(FoxVal* klass) {
    if (klass->type != FOX_CLASS) return Fox_Nil;
    FoxInstance* i = (FoxInstance*)_alloc_val(sizeof(FoxInstance), FOX_INSTANCE);
    i->klass = (FoxClass*)klass;
    Fox_IncRef(klass);
    i->fields = Fox_Dict_New();
    return (FoxVal*)i;
}

FoxVal* Fox_GetAttr(FoxVal* obj, const char* name) {
    if (obj->type == FOX_INSTANCE) {
        FoxInstance* i = (FoxInstance*)obj;
        FoxVal* key = Fox_String_New(name);

        // 1. Check fields
        FoxVal* val = Fox_Dict_Get(i->fields, key);
        if (val != Fox_Nil) {
            Fox_DecRef(key);
            return val; // Get returns New Ref
        }

        // 2. Check class methods
        FoxVal* method = Fox_Dict_Get(i->klass->methods, key);
        Fox_DecRef(key);

        if (method != Fox_Nil && method->type == FOX_FUNCTION) {
            // Create Bound Method
            FoxBoundMethod* bm = (FoxBoundMethod*)_alloc_val(sizeof(FoxBoundMethod), FOX_BOUND_METHOD);
            bm->instance = i;
            Fox_IncRef(obj);
            bm->func = (FoxFunction*)method;
            return (FoxVal*)bm;
        }

        return Fox_Nil;
    }
    return Fox_Nil;
}

void Fox_SetAttr(FoxVal* obj, const char* name, FoxVal* val) {
    if (obj->type == FOX_INSTANCE) {
        FoxInstance* i = (FoxInstance*)obj;
        FoxVal* key = Fox_String_New(name);
        Fox_Dict_Set(i->fields, key, val);
        Fox_DecRef(key);
    }
}

FoxVal* Fox_Call(FoxVal* callable, int argc, FoxVal** argv) {
    if (callable->type == FOX_FUNCTION) {
        FoxFunction* f = (FoxFunction*)callable;
        return f->func_ptr(argc, argv);
    }

    if (callable->type == FOX_CLASS) {
        // Constructor
        return Fox_Instance_New(callable);
    }

    if (callable->type == FOX_BOUND_METHOD) {
        FoxBoundMethod* bm = (FoxBoundMethod*)callable;
        // Prepend 'ini' (instance) to argv
        // We need a new argv array
        FoxVal** new_argv = (FoxVal**)malloc(sizeof(FoxVal*) * (argc + 1));
        new_argv[0] = (FoxVal*)bm->instance;
        for (int i=0; i<argc; i++) new_argv[i+1] = argv[i];

        FoxVal* res = bm->func->func_ptr(argc + 1, new_argv);
        free(new_argv);
        return res;
    }

    return Fox_Nil;
}

bool Fox_IsInstance(FoxVal* obj, FoxVal* klass) {
    if (!obj || !klass) return false;
    if (obj->type != FOX_INSTANCE) return false;
    if (klass->type != FOX_CLASS) return false;
    return ((FoxInstance*)obj)->klass == (FoxClass*)klass;
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
}
