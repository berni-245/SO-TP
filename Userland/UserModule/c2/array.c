#include <array.h>
#include <stdlib.h>
#include <syscalls.h>
#include <utils.h>

typedef struct ArrayCDT {
  // Use uint8_t* instead of void* because void* can't be used in arithmetic operations.
  uint8_t* array;
  unsigned long elementSize;
  unsigned long capacity;
  unsigned long length;
  PrintEleFn printEleFn;
  FreeEleFn freeEleFn;
} ArrayCDT;

void exitWithError(const char* msg);
void copynEleAt(Array a, unsigned long idx, const void* eleArr, unsigned long n);
void copyEleAt(Array a, unsigned long idx, const void* ele);
void growTo(Array a, unsigned long newCapacity);
void growBy(Array a, unsigned long extraCapacity);

void* Array_initialize(
    unsigned long elementSize, unsigned long initialCapacity, PrintEleFn printEleFn, FreeEleFn freeEleFn
) {
  if (elementSize == 0) exitWithError("@Array_initialize elementSize can't be 0");
  ArrayCDT* a = sysMalloc(sizeof(ArrayCDT));
  if (a == NULL) exitWithError("@Array_initialize malloc error");
  a->capacity = initialCapacity ? initialCapacity : 1;
  a->array = sysMalloc(a->capacity * elementSize);
  if (a->array == NULL) {
    sysFree(a);
    exitWithError("@Array_initialize malloc error");
  }
  a->length = 0;
  a->elementSize = elementSize;
  a->printEleFn = printEleFn;
  a->freeEleFn = freeEleFn;
  return a;
}

void Array_free(Array a) {
  if (a == NULL) exitWithError("@Array_free Array instance can't be NULL");

  if (a->freeEleFn != NULL) {
    for (int i = 0; i < a->length; ++i) a->freeEleFn(Array_get(a, i));
  }
  sysFree(a->array);
  sysFree(a);
}

void Array_push(Array a, const void* ele) {
  if (a == NULL) {
    exitWithError("@Array_push Array instance can't be NULL");
  } else if (ele == NULL) exitWithError("@Array_push element to push can't be NULL");

  if (a->length >= a->capacity) growBy(a, a->capacity);

  copyEleAt(a, a->length, ele);
  ++a->length;
}

void Array_setn(Array a, unsigned long idx, void* eleArray, unsigned long length) {
  if (a == NULL) exitWithError("@Array_setn Array instance can't be NULL");
  if (idx >= a->length) exitWithError("@Array_setn idx outside of bounds");
  unsigned long extraCapacity = length - (a->length - idx);
  if (extraCapacity > 0) growBy(a, extraCapacity);
  copynEleAt(a, idx, eleArray, length);
}

bool Array_popGetEle(Array a, void* ele) {
  if (a == NULL) exitWithError("@Array_pop Array instance can't be NULL");
  if (a->length == 0) return false;

  void* eleToPop = Array_get(a, -1);
  sysMemcpy(ele, eleToPop, a->elementSize);
  if (a->freeEleFn != NULL) sysFree(eleToPop);
  --a->length;
  return true;
}

void Array_pop(Array a) {
  if (a == NULL) exitWithError("@Array_pop Array instance can't be NULL");
  if (a->length == 0) return;
  if (a->freeEleFn != NULL) sysFree(Array_get(a, -1));
  --a->length;
}

void Array_clear(Array a) {
  if (a == NULL) exitWithError("@Array_clear Array instance can't be NULL");
  if (a->freeEleFn != NULL) {
    for (int i = 0; i < a->length; ++i) {
      sysFree(Array_get(a, i));
    }
  }
  a->length = 0;
}

unsigned long Array_getLen(Array a) {
  if (a == NULL) exitWithError("@Array_getLen Array instance can't be NULL");
  return a->length;
}

void* Array_get(Array a, long idx) {
  if (a == NULL) exitWithError("@Array_get Array instance can't be NULL");
  if (idx >= a->length) exitWithError("@Array_get idx outside of bounds");
  if (idx < 0) {
    // Note: we negate idx because a->length is unsigned
    if (-idx > a->length) exitWithError("@Array_get idx outside of bounds");
    idx += a->length;
  }
  return a->array + idx * a->elementSize;
}

void Array_set(Array a, long idx, void* ele) {
  if (a == NULL) exitWithError("@Array_set Array instance can't be NULL");
  if (idx >= a->length) exitWithError("@Array_set idx outside of bounds");
  if (idx < 0) {
    if (-idx > a->length) exitWithError("@Array_set idx outside of bounds");
    idx += a->length;
  }

  copyEleAt(a, idx, ele);
}

void Array_print(Array a) {
  if (a == NULL) exitWithError("@Array_print Array instance can't be NULL");
  if (a->printEleFn == NULL) exitWithError("@Array_print call without having set an element print function");
  printf("[ ");
  for (int i = 0; i < Array_getLen(a); ++i) {
    a->printEleFn(Array_get(a, i));
    if (i < Array_getLen(a) - 1) printf(", ");
  }
  printf(" ]\n");
}

void Array_printInfo(Array a) {
  if (a == NULL) exitWithError("@Array_printInfo Array instance can't be NULL");
  printf("{ \n");
  printf(
      "  length: %li\n"
      "  elementSize: %li\n"
      "  capacity: %li\n"
      "  array: %p\n",
      a->length, a->elementSize, a->capacity, a->array
  );
  printf("}\n");
}

Array Array_map(Array a, MapFn mapFn, unsigned long newElemSize, PrintEleFn newPrintEleFn, FreeEleFn freeFn) {
  if (a == NULL) exitWithError("@Array_map Array instance can't be NULL");
  if (mapFn == NULL) exitWithError("@Array_map map function can't be NULL");
  Array mapped = Array_initialize(newElemSize, a->length, newPrintEleFn, freeFn);
  for (int i = 0; i < a->length; ++i) {
    void* mappedEle = sysMalloc(newElemSize);
    if (mappedEle == NULL) exitWithError("@Array_map malloc error");

    mapFn(mappedEle, Array_get(a, i), i);
    Array_push(mapped, mappedEle);
    sysFree(mappedEle);
  }
  return mapped;
}

Array Array_fromVanillaArray(
    const void* array, unsigned long length, unsigned long elementSize, PrintEleFn printEleFn, FreeEleFn freeFn
) {
  Array a = Array_initialize(elementSize, length, printEleFn, freeFn);
  a->length = length;
  copynEleAt(a, 0, array, length);
  return a;
}

void Array_concat(Array dst, Array src) {
  if (dst == NULL || src == NULL) exitWithError("@Array_concat Array instances can't be NULL");
  if (dst->elementSize != src->elementSize) {
    exitWithError("@Array_concat both Array instances should be of same element type");
  }

  unsigned long neededCapacity = dst->length + src->length;
  if (dst->capacity < neededCapacity) growTo(dst, neededCapacity);
  copynEleAt(dst, dst->length, src->array, src->length);
  dst->length += src->length;
}

const void* Array_getVanillaArray(Array a) {
  if (a == NULL) exitWithError("@Array_getVanillaArray Array instance can't be NULL");
  return a->array;
}

void* Array_copyVanillaArrayInto(Array a, void* array) {
  if (a == NULL) exitWithError("@Array_copyVanillaArrayInto Array instance can't be NULL");
  sysMemcpy(array, a->array, a->length * a->elementSize);
  return array;
}

void* Array_getVanillaArrayCopy(Array a) {
  if (a == NULL) exitWithError("@Array_getVanillaArrayCopy Array instance can't be NULL");
  void* array = sysMalloc(a->length * a->elementSize);
  Array_copyVanillaArrayInto(a, array);
  return array;
}

//////////////////////////// Internal Functions ////////////////////////////

void copynEleAt(Array a, unsigned long idx, const void* eleArr, unsigned long n) {
  sysMemcpy(a->array + a->elementSize * idx, eleArr, a->elementSize * n);
}

void copyEleAt(Array a, unsigned long idx, const void* ele) {
  copynEleAt(a, idx, ele, 1);
}

void growTo(Array a, unsigned long newCapacity) {
  void* aux = shittyRealloc(a->array, a->capacity * a->elementSize, newCapacity * a->elementSize);
  if (aux == NULL) exitWithError("@Array's internal func growTo realloc error");
  a->capacity = newCapacity;
  a->array = aux;
}

void growBy(Array a, unsigned long extraCapacity) {
  growTo(a, a->capacity + extraCapacity);
}

void exitWithError(const char* msg) {
  printf("%s\n", msg);
  sysExit(1);
}
