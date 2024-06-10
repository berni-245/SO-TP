#include <shellUtils.h>
#include <utils.h>

#define PHILO_AMOUNT 4
#define PHILO_MAX_AMOUNT 10
#define PHILO_MIN_AMOUNT 2

#define PHYLO_MUTEX "blockAll"
#define PHILO_NAME_LENGTH 5

int philos_on_table;
char* philo_name[PHILO_MAX_AMOUNT];
int philo_state[PHILO_MAX_AMOUNT];
int forks[PHILO_MAX_AMOUNT];
uint64_t pids[PHILO_MAX_AMOUNT];
int change_philos_sem;

enum phylo { EATING, HUNGRY, THINKING, NOTHING };

int right_fork(int i) {
  return (i + 1) % philos_on_table;
}
int left_fork(int i) {
  return (i - 1) % philos_on_table;
}
void blockAll() {
  for (int i = 0; i < philos_on_table; ++i) {
    sysWaitSem(change_philos_sem);
  }
}

void freeAll() {
  for (int i = 0; i < philos_on_table; ++i) {
    sysPostSem(change_philos_sem);
  }
}

void monitor() {
  for (int i = 0; i < philos_on_table; i++) {
    printf("%c ", philo_state[i] ? '.' : 'E');
  }
  printf("\n");
}

void take_forks(int i) {
  philo_state[i] = HUNGRY;
  if (i % 2) {
    sysWaitSem(forks[right_fork(i)]);
    sysWaitSem(forks[i]);
  } else {
    sysWaitSem(forks[i]);
    sysWaitSem(forks[right_fork(i)]);
  }
}
void leave_forks(int i) {
  sysPostSem(forks[i]);
  sysPostSem(forks[right_fork(i)]);
  philo_state[i] = NOTHING;
}

void think(int i) {
  philo_state[i] = THINKING;
  sysSleep(randBetween(400, 800));
}
void eat(int i) {
  philo_state[i] = EATING;
  sysSleep(randBetween(400, 800));
  monitor();
}

void my_phylo_process(uint64_t argc, char* argv[argc]) {
  int n = strToInt(argv[1]);
  if (n < 0) {
    sysExit(ILLEGAL_ARGUMENT);
  }

  while (1) {
    sysWaitSem(change_philos_sem);
    think(n);
    take_forks(n);
    eat(n);
    leave_forks(n);
    sysPostSem(change_philos_sem);
  }
  sysExit(SUCCESS);
}

int check_phylo_pos(int pos) {
  return (pos > PHILO_MAX_AMOUNT || pos < PHILO_MIN_AMOUNT);
}

int add_phylo(int pos) {
  if (check_phylo_pos(pos)) return -1;
  blockAll();
  char sem_name[NAME_MAX_LEN] = "ph_a";
  sem_name[3] = sem_name[3] + pos;
  forks[pos] = sysCreateSemaphore(sem_name, 1);
  if (forks[pos] == -1) {
    printf("Failed creating semaphore \n");
    sysExit(PROCESS_FAILURE);
  }
  philo_name[pos] = sysMalloc(MAX_NAME_LENGTH);
  strcpy(philo_name[pos], sem_name);
  philo_state[pos] = THINKING;
  char phylo_num[3];
  uintToBase(pos, phylo_num, 10);
  const char* argvPhylo[] = {"my_phylo_process", phylo_num};
  printf("Philosopher number %d has joined the game\n", pos);
  philos_on_table++;
  freeAll();
  pids[pos] = sysCreateProcess(sizeof(argvPhylo) / sizeof(argvPhylo[0]), argvPhylo, my_phylo_process);
  return 0;
}
int rem_phylo(int pos) {
  if (check_phylo_pos(pos)) return -1;
  blockAll();
  if (philo_state[pos] == EATING) {
    freeAll();
  }
  philos_on_table--;
  if (!sysDestroySemaphoreByName(philo_name[pos])) printf("Error destroying Philosopher´s semaphore.\n");
  sysFree(philo_name[pos]);
  sysKill(pids[pos]);
  freeAll();
  printf("Philosopher number %d has left the game\n", philos_on_table);
  return 0;
}
void end_phylos() {
  blockAll();
  while (philos_on_table > 0) {
    if (!sysDestroySemaphoreByName(philo_name[philos_on_table - 1]) || !sysKill(pids[philos_on_table - 1])) {
      printf("Error deleting phylo\n");
      sysExit(PROCESS_FAILURE);
    }
    philos_on_table--;
    printf("Philosopher number %d has left the game\n", philos_on_table);
  }
  if (!sysDestroySemaphoreByName(PHYLO_MUTEX)) {
    printf("Error deleting mutex\n");
    sysExit(PROCESS_FAILURE);
  }
  printf("Phylo has been ended\n");
  sysExit(SUCCESS);
}
void commandPhylo(int argc, char* argv[argc]) {
  if (argc != 1) {
    sysExit(TOO_MANY_ARGUMENTS);
  }
  char sem_name[NAME_MAX_LEN] = "ph_a";

  for (int i = 0; i < PHILO_AMOUNT; i++) {
    forks[i] = sysCreateSemaphore(sem_name, 1);
    if (forks[i] == -1) {
      printf("Error creating semaphore: %s\n", sem_name);
      sysExit(PROCESS_FAILURE);
    }
    philo_name[i] = sysMalloc(MAX_NAME_LENGTH);
    strcpy(philo_name[i], sem_name);
    sem_name[3] = sem_name[3] + 1;
  }

  change_philos_sem = sysCreateSemaphore(PHYLO_MUTEX, PHILO_AMOUNT);
  for (int i = 0; i < PHILO_AMOUNT; i++) {
    char philo_num[3];
    uintToBase(i, philo_num, 10);
    const char* argvPhylo[] = {"my_phylo_process", philo_num};
    philo_state[i] = THINKING;
    pids[i] = sysCreateProcess(sizeof(argvPhylo) / sizeof(argvPhylo[0]), argvPhylo, my_phylo_process);
    philos_on_table++;
    printf("Philosopher number %d has joined the game\n", i);
  }

  KeyStruct key;
  while (1) {
    sysHalt();
    if (getKey(&key) != EOF) {
      if (key.character == 'a' || key.character == 'A') {
        add_phylo(philos_on_table);
      } else if (key.character == 'r' || key.character == 'R') {
        rem_phylo(philos_on_table - 1);
      } else if (key.character == 'e' || key.character == 'E') {
        end_phylos();
      }
    }
  }
}
