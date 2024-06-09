#ifndef ARRAY_ADT_H
#define ARRAY_ADT_H

typedef struct ArrayCDT* Array;

#include <stdbool.h>

typedef void (*FreeEleFn)(void* ele);
typedef void (*PrintEleFn)(void* ele);
typedef void (*MapFn)(void* mappedEle, void* ele, unsigned long idx);

void* Array_initialize(
    unsigned long elementSize, unsigned long initialCapacity, FreeEleFn freeFn, PrintEleFn printEleFn
);
void Array_free(Array a);
void Array_push(Array a, const void* ele);
bool Array_popGetEle(Array a, void* ele);
void Array_pop(Array a);
void Array_clear(Array a);
unsigned long Array_getLen(Array a);
void* Array_get(Array a, long idx);
void Array_setn(Array a, long idx, const void* eleArray, unsigned long length);
void Array_set(Array a, long idx, void* ele);
void Array_print(Array a);
void Array_printInfo(Array a);
Array Array_map(Array a, MapFn mapFn, unsigned long newElemSize, PrintEleFn newPrintEleFn, FreeEleFn freeFn);
Array Array_fromVanillaArray(
    const void* array, unsigned long length, unsigned long elementSize, PrintEleFn printEleFn, FreeEleFn freeFn
);
void Array_concat(Array dst, Array src);
const void* Array_getVanillaArray(Array a);
void* Array_copyVanillaArrayInto(Array a, void* array);
void* Array_getVanillaArrayCopy(Array a);

#endif
