#include <shellUtils.h>
#include <testUtil.h>

///////////// Semaphores stuff /////////////

int64_t globalForSemTest;
#define SEM_NAME "sem"
#define TOTAL_PAIR_PROCESSES 2

void slowInc(int64_t* p, int64_t inc) {
  int64_t aux = *p;
  commandChangeProcess(); // This makes the race condition highly probable
  aux += inc;
  *p = aux;
}

void my_process_inc(uint64_t argc, char* argv[argc]) {
  if (argc != 4) {
    sysExit(MISSING_ARGUMENTS);
  }

  int n = strToInt(argv[1]);
  int inc = strToInt(argv[2]);
  int use_sem = strToInt(argv[3]);

  if (n <= 0 || inc == 0 || use_sem < 0) {
    sysExit(ILLEGAL_ARGUMENT);
  }

  if (use_sem) {
    int sem = sysOpenSem(SEM_NAME, 1);
    if (sem < 0) {
      printf("my_process_inc: ERROR opening semaphore\n");
      sysExit(MISSING_ARGUMENTS);
    }
    for (int i = 0; i < n; i++) {
      sysWaitSem(sem);
      slowInc(&globalForSemTest, inc);
      sysPostSem(sem);
    }
  } else {
    for (int i = 0; i < n; i++) slowInc(&globalForSemTest, inc);
  }
  printf("Final value in process: %li\n", globalForSemTest);
  sysExit(SUCCESS);
}

void commandTestSem(int argc, char* argv[argc]) {
  if (argc != 4) {
    printf("Usage: %s <n> <use_sem> <expected_val>\n", argv[0]);
    printf("\tn: number of iterations for each process.\n");
    printf("\tuse_sem: 0 for no semaphores, not 0 to use semaphores.\n");
    printf("\expected_val: initial value for global variable, it should remain the same at the end of the process.\n");
    sysExit(MISSING_ARGUMENTS);
  }

  uint64_t pids[2 * TOTAL_PAIR_PROCESSES];

  const char* argvDec[] = {"my_process_dec", argv[1], "-1", argv[2]};
  const char* argvInc[] = {"my_process_inc", argv[1], "1", argv[2]};

  globalForSemTest = satoi(argv[3]);
  int sem = sysCreateSemaphore(SEM_NAME, 1);
  for (int i = 0; i < TOTAL_PAIR_PROCESSES; i++) {
    pids[i] = sysCreateProcess(sizeof(argvDec) / sizeof(argvDec[0]), argvDec, my_process_inc);
    pids[i + TOTAL_PAIR_PROCESSES] = sysCreateProcess(sizeof(argvDec) / sizeof(argvDec[0]), argvInc, my_process_inc);
  }

  for (int i = 0; i < TOTAL_PAIR_PROCESSES; i++) {
    sysWaitPid(pids[i]);
    sysWaitPid(pids[i + TOTAL_PAIR_PROCESSES]);
  }

  printf("Final value: %li\n", globalForSemTest);
  sysDestroySemaphore(sem);
  sysExit(SUCCESS);
}
