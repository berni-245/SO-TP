#ifndef ARRAY_ADT_H
#define ARRAY_ADT_H

typedef struct ArrayCDT* Array;

#include <stdbool.h>
#include <stdint.h>

typedef void (*FreeEleFn)(void* ele);
typedef void (*MapFn)(void* mappedEle, void* ele, uint64_t idx);

void* arrayInitialize(uint64_t elementSize, uint64_t initialCapacity, FreeEleFn freeEleFn);
bool arrayFree(Array a);
int64_t arrayPush(Array a, const void* ele);
bool arrayPopGetEle(Array a, void* ele);
bool arrayPop(Array a);
bool arrayClear(Array a);
uint64_t arrayGetLen(Array a);
void* arrayGet(Array a, long idx);
bool arraySetn(Array a, long idx, const void* eleArray, uint64_t length);
bool arraySet(Array a, long idx, void* ele);
Array arrayFromVanillaArray(const void* array, uint64_t length, uint64_t elementSize, FreeEleFn freeFn);
bool arrayConcat(Array dst, Array src);
const void* arrayGetVanillaArray(Array a);
void* arrayCopyVanillaArrayInto(Array a, void* array);
void* Array_getVanillaArrayCopy(Array a);

#endif
