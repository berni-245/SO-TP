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
  int writeIdx;
  int readIdx;
  sem_t mutex;
  sem_t emptyCountSem;
  sem_t writtenCountSem;
  bool destroyed;
  // PCB* readerPcb;
  // PCB* writerPcb;
} Pipe;

void initializePipes();
long pipeInit();
long readPipe(int pipeId, char* buf, int len);
long writePipe(int pipeId, const char* buf, int len);
bool destroyPipe(int pipeId);
void writeStdin(char c);

#endif
