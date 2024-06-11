#ifndef ARRAY_ADT_H
#define ARRAY_ADT_H

typedef struct ArrayCDT* Array;

#include <stdbool.h>
#include <stdint-gcc.h>

typedef void (*FreeEleFn)(void* ele);
typedef void (*PrintEleFn)(void* ele);
typedef void (*MapFn)(void* mappedEle, void* ele, unsigned long idx);

void* Array_initialize(
    unsigned long elementSize, unsigned long initialCapacity, FreeEleFn freeFn, PrintEleFn printEleFn
);
void arrayFree(Array a);
void arrayPush(Array a, const void* ele);
bool arrayPopGetEle(Array a, void* ele);
void arrayPop(Array a);
void arrayClear(Array a);
unsigned long arrayGetLen(Array a);
void* arrayGet(Array a, long idx);
void arraySetn(Array a, long idx, const void* eleArray, uint64_t length);
void arraySet(Array a, long idx, void* ele);
void Array_print(Array a);
void Array_printInfo(Array a);
Array Array_map(Array a, MapFn mapFn, unsigned long newElemSize, PrintEleFn newPrintEleFn, FreeEleFn freeFn);
Array Array_fromVanillaArray(
    const void* array, unsigned long length, unsigned long elementSize, PrintEleFn printEleFn, FreeEleFn freeFn
);
void arrayConcat(Array dst, Array src);
const void* arrayGetVanillaArray(Array a);
void* arrayCopyVanillaArrayInto(Array a, void* array);
void* Array_getVanillaArrayCopy(Array a);

#endif
