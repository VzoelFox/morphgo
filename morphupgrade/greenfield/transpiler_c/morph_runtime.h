#ifndef MORPH_RUNTIME_H
#define MORPH_RUNTIME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// --- Core Types ---
typedef enum {
    FOX_NIL,
    FOX_INT,
    FOX_FLOAT,
    FOX_STRING,
    FOX_BOOL,
    FOX_LIST,
    FOX_DICT,
    FOX_FUNCTION,    // C Function Wrapper
    FOX_CLASS,
    FOX_INSTANCE,
    FOX_BOUND_METHOD // (instance + function)
} FoxType;

typedef struct FoxVal {
    FoxType type;
    int ref_count;
} FoxVal;

typedef struct {
    FoxVal base;
    long long value;
} FoxInt;

typedef struct {
    FoxVal base;
    double value;
} FoxFloat;

typedef struct {
    FoxVal base;
    bool value;
} FoxBool;

typedef struct {
    FoxVal base;
    char* data;
    size_t length;
} FoxString;

typedef struct {
    FoxVal base;
    FoxVal** elements;
    size_t count;
    size_t capacity;
} FoxList;

// Hash Map Entry
typedef struct FoxMapEntry {
    char* key;
    FoxVal* value;
    struct FoxMapEntry* next;
} FoxMapEntry;

typedef struct {
    FoxVal base;
    FoxMapEntry** buckets;
    size_t bucket_count;
    size_t item_count;
} FoxDict;

// --- Function & Class Types ---

// Generic Function Pointer Type
typedef FoxVal* (*FoxCFunc)(int argc, FoxVal** argv);

typedef struct {
    FoxVal base;
    FoxCFunc func_ptr;
    char* name;
} FoxFunction;

typedef struct {
    FoxVal base;
    char* name;
    FoxVal* methods; // FoxDict
} FoxClass;

typedef struct {
    FoxVal base;
    FoxClass* klass;
    FoxVal* fields; // FoxDict
} FoxInstance;

typedef struct {
    FoxVal base;
    FoxInstance* instance;
    FoxFunction* func;
} FoxBoundMethod;

// --- Global Constants ---
extern FoxVal* Fox_Nil;
extern FoxVal* Fox_True;
extern FoxVal* Fox_False;

// --- API ---
void Fox_InitRuntime();
void Fox_ShutdownRuntime();

FoxVal* Fox_Int_New(long long val);
FoxVal* Fox_String_New(const char* val);
void Fox_Print(FoxVal* val);
void Fox_DecRef(FoxVal* val);
void Fox_IncRef(FoxVal* val);
bool Fox_IsTrue(FoxVal* val);
bool Fox_IsString(FoxVal* val);
char* Fox_AsCString(FoxVal* val);

// --- Arithmetic ---
FoxVal* Fox_Add(FoxVal* a, FoxVal* b);
FoxVal* Fox_Sub(FoxVal* a, FoxVal* b);
FoxVal* Fox_Mul(FoxVal* a, FoxVal* b);
FoxVal* Fox_Div(FoxVal* a, FoxVal* b);

// --- Comparison ---
FoxVal* Fox_Eq(FoxVal* a, FoxVal* b);
FoxVal* Fox_Lt(FoxVal* a, FoxVal* b);
FoxVal* Fox_Gt(FoxVal* a, FoxVal* b);

// --- List API ---
FoxVal* Fox_List_New(size_t capacity);
void Fox_List_Append(FoxVal* list, FoxVal* item);
FoxVal* Fox_List_Get(FoxVal* list, int index);

// --- Dict API ---
FoxVal* Fox_Dict_New();
void Fox_Dict_Set(FoxVal* dict, FoxVal* key, FoxVal* value);
FoxVal* Fox_Dict_Get(FoxVal* dict, FoxVal* key);

// --- Object API ---
FoxVal* Fox_Function_New(FoxCFunc func, const char* name);
FoxVal* Fox_Class_New(const char* name);
void Fox_Class_AddMethod(FoxVal* klass, const char* name, FoxVal* func);
FoxVal* Fox_Instance_New(FoxVal* klass);
FoxVal* Fox_GetAttr(FoxVal* obj, const char* name);
void Fox_SetAttr(FoxVal* obj, const char* name, FoxVal* val);
FoxVal* Fox_Call(FoxVal* callable, int argc, FoxVal** argv);

// --- Generic Access ---
FoxVal* Fox_GetItem(FoxVal* obj, FoxVal* key);
void Fox_SetItem(FoxVal* obj, FoxVal* key, FoxVal* val);

#endif
