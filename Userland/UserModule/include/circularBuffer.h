#ifndef CICRULAR_BUFFER_H
#define CICRULAR_BUFFER_H

#include <stdint.h>

void incCircularIdxBy(int32_t* idx, int32_t val, int32_t max);
void incCircularIdx(int32_t* idx, int32_t max);
void decCircularIdxBy(int32_t* idx, int32_t val, int32_t max);
void decCircularIdx(int32_t* idx, int32_t max);

#endif
