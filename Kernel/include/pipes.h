#ifndef PIPES_H
#define PIPES_H

#define BUFFER_SIZE 40

#include <semaphores.h>
#include <stdint.h>
#include <scheduler.h>

#define stdout 0
#define stdin 1
#define stderr 2

typedef struct {
  char buffer[BUFFER_SIZE];
  int32_t writeIdx;
  int32_t readIdx;
  sem_t mutex;
  sem_t emptyCountSem;
  sem_t writtenCountSem;
  bool destroyed;
  // PCB* readerPcb;
  // PCB* writerPcb;
} Pipe;

void initializePipes();
int64_t pipeInit();
int64_t readPipe(int32_t pipeId, char* buf, int32_t len);
int64_t writePipe(int32_t pipeId, const char* buf, int32_t len);
bool destroyPipe(int32_t pipeId);
void writeStdin(char c);
int64_t readStdin(char* buf, int32_t len);

#endif
