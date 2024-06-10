#include <shellUtils.h>
#include <stdlib.h>
#include <syscalls.h>
#include <test_util.h>

#define MAX_BLOCKS 10

typedef struct MM_rq {
  void* address;
  uint32_t size;
} mm_rq;

void commandTestMM(int argc, char* argv[]) {

  mm_rq mm_rqs[MAX_BLOCKS];
  uint8_t rq;
  uint32_t total;
  int64_t max_memory;

  if (argc < 2) {
    printf("Usage: %s <max_memory>\n", argv[0]);
    puts("\tmax_memory: positive integer");
    sysExit(MISSING_ARGUMENTS);
  }

  if ((max_memory = strToInt(argv[1])) <= 0) {
    printf("Usage: %s <max_memory>\n", argv[0]);
    puts("\tmax_memory: positive integer");
    sysExit(ILLEGAL_ARGUMENT);
  }

  max_memory = max_memory * (1 << 20);
  while (1) {
    rq = 0;
    total = 0;

    // Request as many blocks as we can
    while (rq < MAX_BLOCKS && total < max_memory) {
      sysSleep(500);
      mm_rqs[rq].size = GetUniform(max_memory - total - 1) + 1;
      mm_rqs[rq].address = sysMalloc(mm_rqs[rq].size);

      if (mm_rqs[rq].address != NULL) {
        total += mm_rqs[rq].size;
        rq++;
      }
    }

    // Set
    uint32_t i;
    for (i = 0; i < rq; i++)
      if (mm_rqs[i].address != NULL) setMem(mm_rqs[i].address, i, mm_rqs[i].size);

    // Check
    for (i = 0; i < rq; i++)
      if (mm_rqs[i].address != NULL)
        if (!memcheck(mm_rqs[i].address, i, mm_rqs[i].size)) {
          printf("test_mm ERROR\n");
          sysExit(OUT_OF_BOUNDS);
        }

    // Free
    for (i = 0; i < rq; i++)
      if (mm_rqs[i].address != NULL) sysFree(mm_rqs[i].address);
  }
}
