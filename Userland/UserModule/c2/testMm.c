#include <shellUtils.h>
#include <stdlib.h>
#include <syscalls.h>
#include <testUtil.h>
#include <utils.h>

#define MAX_BLOCKS 10

typedef struct MM_rq {
  void* address;
  uint32_t size;
} mm_rq;

static char* usageMessage = "Usage: %s <max_memory>\n\tmax_memory: maximum number of KB to allocate\n";

void commandTestMM(int argc, char* argv[]) {

  mm_rq mm_rqs[MAX_BLOCKS];
  uint8_t rq;
  uint32_t total;
  int64_t max_memory;

  if (argc < 2) {
    printf(usageMessage, argv[0]);
    sysExit(MISSING_ARGUMENTS);
  }

  if ((max_memory = strToInt(argv[1])) <= 0) {
    printf(usageMessage, argv[0]);
    sysExit(ILLEGAL_ARGUMENT);
  }

  int myPid = sysGetPid();
  char pidStr[200];
  uintToBase(myPid, pidStr, 10);
  char name[206] = "mem_p";
  strcpy(name + 5, pidStr);
  const char* argvMem[] = {name, pidStr};

  max_memory = max_memory * (1 << 10);
  while (1) {
    rq = 0;
    total = 0;

    // Request as many blocks as we can
    while (rq < MAX_BLOCKS && total < max_memory) {
      mm_rqs[rq].size = GetUniform(max_memory - total - 1) + 1;
      mm_rqs[rq].address = sysMalloc(mm_rqs[rq].size);
      if (mm_rqs[rq].address != NULL) {
        total += mm_rqs[rq].size;
        rq++;
      } else {
        printf("Not enough memory to allocate: %lu B\n", mm_rqs[rq].size);
      }
    }
    sysSleep(1000);

    // Set
    uint32_t i;
    for (i = 0; i < rq; i++) {
      if (mm_rqs[i].address != NULL) setMem(mm_rqs[i].address, i, mm_rqs[i].size);
    }

    printf("After allocation:\n");
    int pid = sysCreateProcess(2, argvMem, commandGetMemoryState);
    sysWaitPid(pid);

    // Check
    for (i = 0; i < rq; i++) {
      if (mm_rqs[i].address != NULL)
        if (!memcheck(mm_rqs[i].address, i, mm_rqs[i].size)) {
          printf("test_mm ERROR\n");
          sysExit(OUT_OF_BOUNDS);
        }
    }

    // Free
    for (i = 0; i < rq; i++) {
      if (mm_rqs[i].address != NULL) sysFree(mm_rqs[i].address);
    }

    printf("After free:\n");
    pid = sysCreateProcess(2, argvMem, commandGetMemoryState);
    sysWaitPid(pid);
  }
}
