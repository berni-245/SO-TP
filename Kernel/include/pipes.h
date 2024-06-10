#ifndef PIPES_H
#define PIPES_H

#define BUFFER_SIZE 40

#include <stdint.h>

#define stdout 0
#define stdin 1
#define stderr 2

typedef struct {
  char buffer[BUFFER_SIZE];
  int writeIdx;
  int readIdx;
  int32_t mutex;
  int32_t emptyCount;
  int32_t writtenCount;
} Pipe;

void initializePipes();
uint64_t readPipe(int p, char* buf, int len);
uint64_t writePipe(int p, const char* buf, int len);

#endif
