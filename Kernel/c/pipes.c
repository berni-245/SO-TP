#include <memoryManager.h>
#include <pipes.h>
#include <semaphores.h>
#include <stdbool.h>
#include <stdint.h>
#include <utils.h>

#define MAX_PIPES 20

int pipes_amount;
Pipe pipeArray[MAX_PIPES];

// Hacer a pipeArray un Array dinámico (traer Array lib para kernel tmbn)
// y tener un lista de free pipes indexes. Si la lista está vacía createPipe
// me pushea un nuevo pipe al array. Si hay alguna vacía la reusa (o reusa
// el espacio en el array at least, xq podemos hacer que se haga free en closePipe
// o que queden para siempre creadas y listo).

void initializePipes() {
  char mutex[] = "pipe_m_A";
  char emptySem[] = "pipe_e_A";
  char writtenSem[] = "pipe_f_A";
  for (int i = 0; i < MAX_PIPES; ++i) {
    pipeArray[i].writeIdx = 0;
    pipeArray[i].readIdx = 0;
    pipeArray[i].mutex = createSemaphore(mutex, 1);
    pipeArray[i].emptyCount = createSemaphore(emptySem, BUFFER_SIZE);
    pipeArray[i].writtenCount = createSemaphore(writtenSem, 0);
    ++mutex[7];
    ++emptySem[7];
    ++writtenSem[7];
  }
}

uint64_t readPipe(int p, char* buf, int len) {
  if (p < 0 || p >= MAX_PIPES) return -1;
  if (len <= 0) return 0;
  uint64_t pos = 0;
  // This is wrong. `pipe` would be a copy of the `pipeArray[p]` so modifications
  // to `pipe` wouldn't affect the real one... Also it's too big and it seem to be
  // overwriting stuff from the process pcb.
  // Pipe pipe = pipeArray[p];
  bool reachedEnd = false;
  do {
    waitSemaphore(pipeArray[p].writtenCount);
    waitSemaphore(pipeArray[p].mutex);
    buf[pos++] = pipeArray[p].buffer[pipeArray[p].readIdx];
    pipeArray[p].readIdx = (pipeArray[p].readIdx + 1) % BUFFER_SIZE;
    if (pipeArray[p].readIdx == pipeArray[p].writeIdx) reachedEnd = true;
    postSemaphore(pipeArray[p].mutex);
    postSemaphore(pipeArray[p].emptyCount);
  } while (pos < len && !reachedEnd);

  return pos;
}

uint64_t writePipe(int p, const char* buf, int len) {
  if (p < 0 || p >= MAX_PIPES) return -1;
  uint64_t pos = 0;
  // Pipe pipe = pipeArray[p];
  while (pos < len) {
    waitSemaphore(pipeArray[p].emptyCount);
    waitSemaphore(pipeArray[p].mutex);
    pipeArray[p].buffer[pipeArray[p].writeIdx] = buf[pos++];
    pipeArray[p].writeIdx = (pipeArray[p].writeIdx + 1) % BUFFER_SIZE;
    postSemaphore(pipeArray[p].mutex);
    postSemaphore(pipeArray[p].writtenCount);
  }
  return pos;
}

// int check_pipe(int pipe) {
//   return (pipe < 0 || pipe >= MAX_PIPES || !pipeArray[pipe].is_used);
// }

// void Pipe_initialize() {
//   for (int i = 0; i < MAX_PIPES; i++) {
//     pipeArray[i].is_used = 0;
//   }
//   pipes_amount = 0;
// }

// int find_available_pipe() {
//   if (pipes_amount >= MAX_PIPES) {
//     return -1;
//   }
//   for (int i = 0; i < MAX_PIPES; ++i) {
//     if (!pipeArray[i].is_used) {
//       return i;
//     }
//   }
//   return -1;
// }

// int find_pipe(const char* name) {
//   if (pipes_amount == 0) return -1;
//   for (int i = 0; i < MAX_PIPES; i++) {
//     if (pipeArray[i].is_used && strcmp(pipeArray[i].pipe->name, name) == 0) {
//       return i;
//     }
//   }
//   return -1;
// }
// Si no quiero asignar el proceso todavía le mando 0 y con el join lo puedo agregar

// int create_pipe() {
// }

// int join_pipe(const char* pipe_name, int process_write, int process_read) {
//   int pipe_id = find_pipe(pipe_name);
//   if (pipe_id == -1) {
//     return -1;
//   }
//   if (pipeArray[pipe_id].pipe->write == 0 && process_write > 0) {
//     pipeArray[pipe_id].pipe->write = process_write;
//   } else if (process_write != 0) {
//     return -1;
//   }
//   if (pipeArray[pipe_id].pipe->read == 0 && process_read > 0) {
//     pipeArray[pipe_id].pipe->read = process_read;
//   } else if (process_read != 0) {
//     return -1;
//   }
//   return pipe_id;
// }

// int write_pipe(int pipe, const char* info, int size) {
//   if (check_pipe(pipe)) {
//     return -1;
//   }
//   if (pipeArray[pipe].pipe->write != 0 && pipeArray[pipe].pipe->read != 0) {
//     int pos = 0;
//     while (pos < size) {
//       waitSemaphore(pipeArray[pipe].pipe->empty);
//       waitSemaphore(pipeArray[pipe].pipe->mutex);
//       pipeArray[pipe].pipe->buffer[pipeArray[pipe].pipe->wirteIdx] = info[pos++];
//       pipeArray[pipe].pipe->wirteIdx = (pipeArray[pipe].pipe->wirteIdx + 1) % BUFFER_SIZE;
//       waitSemaphore(pipeArray[pipe].pipe->mutex);
//       waitSemaphore(pipeArray[pipe].pipe->full);
//     }
//     return pos;
//   }
//   return -1;
// }

// int read_pipe(int pipe, char* info, int size) {
//   if (check_pipe(pipe)) {
//     return -1;
//   }
//   if (pipeArray[pipe].pipe->write != 0 && pipeArray[pipe].pipe->read != 0) {
//     int pos = 0;
//     while (pos < size) {
//       waitSemaphore(pipeArray[pipe].pipe->full);
//       waitSemaphore(pipeArray[pipe].pipe->mutex);
//       info[pos++] = pipeArray[pipe].pipe->buffer[pipeArray[pipe].pipe->readIdx];
//       pipeArray[pipe].pipe->readIdx = (pipeArray[pipe].pipe->readIdx + 1) % BUFFER_SIZE;
//       waitSemaphore(pipeArray[pipe].pipe->mutex);
//       waitSemaphore(pipeArray[pipe].pipe->empty);
//     }
//     return pos;
//   }
//   return -1;
// }

// int destroy_pipe(int pipe) {
//   if (check_pipe(pipe)) {
//     return -1;
//   }
//   free(pipeArray[pipe].pipe->pipeId);
//   free(pipeArray[pipe].pipe);
//   pipeArray[pipe].is_used = 0;
//   pipe_size--;
//   return 0;
// }
