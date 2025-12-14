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
    FOX_DICT
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

// --- Arithmetic ---
FoxVal* Fox_Add(FoxVal* a, FoxVal* b);
FoxVal* Fox_Sub(FoxVal* a, FoxVal* b);
FoxVal* Fox_Mul(FoxVal* a, FoxVal* b);
FoxVal* Fox_Div(FoxVal* a, FoxVal* b);

// --- Comparison ---
FoxVal* Fox_Eq(FoxVal* a, FoxVal* b);
FoxVal* Fox_Lt(FoxVal* a, FoxVal* b);
FoxVal* Fox_Gt(FoxVal* a, FoxVal* b);

#endif
