#ifndef ARRAY_ADT_H
#define ARRAY_ADT_H

typedef struct ArrayCDT* Array;

#include <stdbool.h>
#include <stdint.h>

typedef void (*FreeEleFn)(void* ele);
typedef int (*CompareEleFn)(void* ele1, void* ele2);
typedef void (*MapFn)(void* mappedEle, void* ele, unsigned long idx);

void* Array_initialize(unsigned long elementSize, unsigned long initialCapacity, FreeEleFn freeEleFn, CompareEleFn cmpEleFn);
void Array_free(Array a);
void Array_push(Array a, const void* ele);
bool Array_popGetEle(Array a, void* ele);
void Array_pop(Array a);
void Array_clear(Array a);
unsigned long Array_getLen(Array a);
void* Array_get(Array a, long idx);
void Array_getnCopy(Array a, int64_t idx, uint64_t n, void* eleArr);
void Array_setn(Array a, long idx, const void* eleArray, uint64_t length, bool free);
void Array_set(Array a, long idx, void* ele);
void Array_printInfo(Array a);
void Array_concat(Array dst, Array src);
const void* Array_getVanillaArray(Array a);
void Array_copyVanillaArrayInto(Array a, void* eleArr);
void* Array_getVanillaArrayCopy(Array a);
bool Array_equals(Array a1, Array a2);
int Array_find(Array a, void* ele);
bool Array_has(Array a, void* ele);
void Array_remove(Array a, int64_t idx);
uint64_t Array_getEleSize(Array a);

#endif
