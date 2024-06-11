#include <syscalls.h>
#include <testUtil.h>
#include <stdint.h>
#include <shellUtils.h>

enum State { RUNNING,
             BLOCKED,
             KILLED };

typedef struct P_rq {
  int32_t pid;
  enum State state;
} p_rq;

void commandTestProcesses(int32_t argc, char *argv[]) {
  uint8_t rq;
  uint8_t alive = 0;
  uint8_t action;
  uint64_t max_processes;
  const char *argvAux[] = {"endless_loop"};

  if (argc != 2) {
    printf("Usage: %s <max_processes>\n", argv[0]);
    sysExit(MISSING_ARGUMENTS);
  }

  if ((max_processes = satoi(argv[1])) <= 0) {
    printf("Usage: %s <max_processes>\n", argv[0]);
    sysExit(ILLEGAL_ARGUMENT);
  }

  p_rq p_rqs[max_processes];


  while (1) {

    // Create max_processes processes
    for (rq = 0; rq < max_processes; rq++) {
      p_rqs[rq].pid = sysCreateProcess(1, argvAux, endless_loop);

      if (p_rqs[rq].pid == -1) {
        printf("test_processes: ERROR creating process\n");
        sysExit(PROCESS_FAILURE);
      } else {
        p_rqs[rq].state = RUNNING;
        alive++;
      }

    }

    // Randomly kills, blocks or unblocks processes until every one has been killed
    while (alive > 0) {

      for (rq = 0; rq < max_processes; rq++) {
        action = GetUniform(100) % 2;

        switch (action) {
          case 0:
            if (p_rqs[rq].state == RUNNING || p_rqs[rq].state == BLOCKED) {
              if(!sysKill(p_rqs[rq].pid)) {
                printf("Process couldn´t be killed");
                sysExit(PROCESS_FAILURE);
              }
              else {
                p_rqs[rq].state = KILLED;
                alive--;
              }
            }
            break;
          case 1:
            if (p_rqs[rq].state == RUNNING) {
              if(!sysBlockedByUser(p_rqs[rq].pid)){
                printf("test_processes: ERROR blocking process\n");
                sysExit(PROCESS_FAILURE);
              }
              p_rqs[rq].state = BLOCKED;
            }
            break;
        }
      }

      // Randomly unblocks processes
      for (rq = 0; rq < max_processes; rq++)
        if (p_rqs[rq].state == BLOCKED && GetUniform(100) % 2) {
            if (!sysUnBlock(p_rqs[rq].pid)) {
              printf("test_processes: ERROR unblocking process\n");
              sysExit(PROCESS_FAILURE);
            }
            p_rqs[rq].state = RUNNING;
        }
    }
    printf("%d processes alive\n", alive);
    printf("Execution of testProcesses ended successfully\n");
    sysExit(SUCCESS);
  }
}
