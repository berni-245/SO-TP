#include <syscalls.h>
#include <test_util.h>
#include <shellUtils.h>


#define MINOR_WAIT 1000000  // TODO: Change this value to prevent a process from flooding the screen
#define WAIT 1000000000      // TODO: Change this value to make the wait long enough to see theese processes beeing run at least twice

#define TOTAL_PROCESSES 3
#define LOWEST 1  // TODO: Change as required
#define MEDIUM 5  // TODO: Change as required
#define HIGHEST 9 // TODO: Change as required

int64_t prio[TOTAL_PROCESSES] = {LOWEST, MEDIUM, HIGHEST};

void commandTestPriority(int argc, char* argv[argc]) {
  if(argc!=2){
    printf("Usage: %s <0: small wait, 1: long wait>\n", argv[0]);
    sysExit(TOO_MANY_ARGUMENTS);
  }
  if(strcmp(argv[1], "0")!=0 && strcmp(argv[1], "1")!=0){
    printf("Usage: %s <0: small wait, 1: long wait>\n", argv[0]);
    sysExit(ILLEGAL_ARGUMENT);
  }
  int64_t pids[TOTAL_PROCESSES];
  const char* argv2[] = {"commandLoop", argv[1]};
  uint64_t i;

  for (i = 0; i < TOTAL_PROCESSES; i++)
    pids[i] = sysCreateProcess(2, argv2, endless_loop_print);

  bussy_wait(WAIT);
  printf("\nCHANGING PRIORITIES...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    sysChangePriority(pids[i], prio[i]);

  bussy_wait(WAIT);
  printf("\nBLOCKING...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    sysBlock(pids[i]);

  printf("CHANGING PRIORITIES WHILE BLOCKED...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    sysChangePriority(pids[i], MEDIUM);

  printf("UNBLOCKING...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    sysUnBlock(pids[i]);

  bussy_wait(WAIT);
  printf("\nKILLING...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    sysKill(pids[i]);
  sysExit(SUCCESS);
}