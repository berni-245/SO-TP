#include <array.h>
#include <stdlib.h>
#include <syscalls.h>
#include <utils.h>

typedef struct ArrayCDT {
  // Use uint8_t* instead of void* because void* can't be used in arithmetic operations.
  uint8_t* array;
  long elementSize;
  long capacity;
  long length;
  PrintEleFn printEleFn;
  FreeEleFn freeEleFn;
} ArrayCDT;

void copynEleAt(Array a, unsigned long idx, const void* eleArr, unsigned long n);
void copyEleAt(Array a, unsigned long idx, const void* ele);
void growTo(Array a, unsigned long newCapacity);
void growBy(Array a, unsigned long extraCapacity);

void* Array_initialize(
    unsigned long elementSize, unsigned long initialCapacity, FreeEleFn freeEleFn, PrintEleFn printEleFn
) {
  if (elementSize == 0) exitWithError("@arrayInitialize elementSize can't be 0");
  ArrayCDT* a = sysMalloc(sizeof(ArrayCDT));
  if (a == NULL) exitWithError("@arrayInitialize malloc error");
  a->capacity = initialCapacity ? initialCapacity : 1;
  a->array = sysMalloc(a->capacity * elementSize);
  if (a->array == NULL) {
    sysFree(a);
    exitWithError("@arrayInitialize malloc error");
  }
  a->length = 0;
  a->elementSize = elementSize;
  a->printEleFn = printEleFn;
  a->freeEleFn = freeEleFn;
  return a;
}

void arrayFree(Array a) {
  if (a == NULL) exitWithError("@arrayFree Array instance can't be NULL");

  if (a->freeEleFn != NULL) {
    for (int i = 0; i < a->length; ++i) a->freeEleFn(arrayGet(a, i));
  }
  sysFree(a->array);
  sysFree(a);
}

void arrayPush(Array a, const void* ele) {
  if (a == NULL) {
    exitWithError("@arrayPush Array instance can't be NULL");
  } else if (ele == NULL) exitWithError("@arrayPush element to push can't be NULL");

  if (a->length >= a->capacity) growBy(a, a->capacity);

  copyEleAt(a, a->length, ele);
  ++a->length;
}

bool arrayPopGetEle(Array a, void* ele) {
  if (a == NULL) exitWithError("@arrayPop Array instance can't be NULL");
  if (a->length == 0) return false;

  void* eleToPop = arrayGet(a, -1);
  sysMemcpy(ele, eleToPop, a->elementSize);
  if (a->freeEleFn != NULL) a->freeEleFn(eleToPop);
  --a->length;
  return true;
}

void arrayPop(Array a) {
  if (a == NULL) exitWithError("@arrayPop Array instance can't be NULL");
  if (a->length == 0) return;
  if (a->freeEleFn != NULL) a->freeEleFn(arrayGet(a, -1));
  --a->length;
}

void arraySetn(Array a, long idx, const void* eleArray, uint64_t length) {
  if (a == NULL) exitWithError("@arraySetn Array instance can't be NULL");
  if (idx >= a->length) exitWithError("@arraySetn idx outside of bounds");
  if (idx < 0) {
    if (-idx > a->length) exitWithError("@arraySetn idx outside of bounds");
    idx += a->length;
  }
  if (a->freeEleFn != NULL) {
    for (int i = idx; i < idx + length; ++i) {
      a->freeEleFn(arrayGet(a, i));
    }
  }
  if (length > a->length - idx) growBy(a, length - (a->length - idx));
  copynEleAt(a, idx, eleArray, length);
}

void arraySet(Array a, long idx, void* ele) {
  arraySetn(a, idx, ele, 1);
}

void* arrayGet(Array a, long idx) {
  if (a == NULL || idx >= a->length) return NULL;
  if (idx < 0) {
    // Note: we negate idx because a->length is unsigned
    if (-idx > a->length) return NULL;
    idx += a->length;
  }
  return a->array + idx * a->elementSize;
}

void arrayClear(Array a) {
  if (a == NULL) exitWithError("@arrayClear Array instance can't be NULL");
  if (a->freeEleFn != NULL) {
    for (int i = 0; i < a->length; ++i) {
      a->freeEleFn(arrayGet(a, i));
    }
  }
  a->length = 0;
}

unsigned long arrayGetLen(Array a) {
  if (a == NULL) exitWithError("@arrayGetLen Array instance can't be NULL");
  return a->length;
}

void Array_print(Array a) {
  if (a == NULL) exitWithError("@Array_print Array instance can't be NULL");
  if (a->printEleFn == NULL) exitWithError("@Array_print call without having set an element print function");
  printf("[ ");
  for (int i = 0; i < arrayGetLen(a); ++i) {
    a->printEleFn(arrayGet(a, i));
    if (i < arrayGetLen(a) - 1) printf(", ");
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
  Array mapped = Array_initialize(newElemSize, a->length, freeFn, newPrintEleFn);
  for (int i = 0; i < a->length; ++i) {
    void* mappedEle = sysMalloc(newElemSize);
    if (mappedEle == NULL) exitWithError("@Array_map malloc error");

    mapFn(mappedEle, arrayGet(a, i), i);
    arrayPush(mapped, mappedEle);
    sysFree(mappedEle);
  }
  return mapped;
}

Array Array_fromVanillaArray(
    const void* array, unsigned long length, unsigned long elementSize, PrintEleFn printEleFn, FreeEleFn freeFn
) {
  Array a = Array_initialize(elementSize, length, freeFn, printEleFn);
  a->length = length;
  copynEleAt(a, 0, array, length);
  return a;
}

void arrayConcat(Array dst, Array src) {
  if (dst == NULL || src == NULL) exitWithError("@arrayConcat Array instances can't be NULL");
  if (dst->elementSize != src->elementSize) {
    exitWithError("@arrayConcat both Array instances should be of same element type");
  }

  unsigned long neededCapacity = dst->length + src->length;
  if (dst->capacity < neededCapacity) growTo(dst, neededCapacity);
  copynEleAt(dst, dst->length, src->array, src->length);
  dst->length += src->length;
}

const void* arrayGetVanillaArray(Array a) {
  if (a == NULL) exitWithError("@arrayGetVanillaArray Array instance can't be NULL");
  return a->array;
}

void* arrayCopyVanillaArrayInto(Array a, void* array) {
  if (a == NULL) exitWithError("@arrayCopyVanillaArrayInto Array instance can't be NULL");
  sysMemcpy(array, a->array, a->length * a->elementSize);
  return array;
}

void* arrayGetVanillaArrayCopy(Array a) {
  if (a == NULL) exitWithError("@arrayGetVanillaArrayCopy Array instance can't be NULL");
  void* array = sysMalloc(a->length * a->elementSize);
  arrayCopyVanillaArrayInto(a, array);
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
