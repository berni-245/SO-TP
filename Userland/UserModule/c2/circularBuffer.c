#include <circularBuffer.h>
#include <stdint.h>

void incCircularIdxBy(int32_t* idx, int32_t val, int32_t len) {
  *idx = (*idx + val) % len;
}

void incCircularIdx(int32_t* idx, int32_t len) {
  incCircularIdxBy(idx, 1, len);
}

void decCircularIdxBy(int32_t* idx, int32_t val, int32_t len) {
  *idx = *idx - val;
  if (*idx < 0) *idx = len + *idx;
}

void decCircularIdx(int32_t* idx, int32_t len) {
  decCircularIdxBy(idx, 1, len);
}
