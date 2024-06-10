#include <shellUtils.h>
#include <utils.h>

#define PHILO_AMOUNT 5
#define PHILO_MAX_AMOUNT 10
#define PHILO_MIN_AMOUNT 2

#define PHYLO_MUTEX "blockAll"

int philos_on_table;
int philo_state[PHILO_MAX_AMOUNT];
int forks[PHILO_MAX_AMOUNT];
int pids[PHILO_MAX_AMOUNT];
int change_philos_sem;

enum phylo { EATING, HUNGRY, THINKING, NOTHING };

int right_fork(int i) {
  return (i + 1) % philos_on_table;
}

void blockAll() {
  for (int i = 0; i < philos_on_table; i++) {
    sysWaitSem(change_philos_sem);
  }
}

void freeAll() {
  for (int i = 0; i < philos_on_table; i++) {
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
  sysSleep(randBetween(600, 800));
}
void eat(int i) {
  philo_state[i] = EATING;
  sysSleep(randBetween(600, 800));
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

void blockAlmostAll() {
  for (int i = 0; i < philos_on_table - 1; i++) {
    sysWaitSem(change_philos_sem);
  }
}
void freeAlmostAll() {
  for (int i = 0; i < philos_on_table - 1; i++) {
    sysPostSem(change_philos_sem);
  }
}
int add_phylo(int pos) {
  if (check_phylo_pos(pos)) return -1;
  blockAll();
  forks[pos] = sysSemInit(1);
  if (forks[pos] == -1) {
    printf("Failed creating semaphore\n");
    freeAll();
    sysExit(PROCESS_FAILURE);
  }
  philo_state[pos] = THINKING;
  char phylo_num[3];
  uintToBase(pos, phylo_num, 10);
  const char* argvPhylo[] = {"my_phylo_process", phylo_num};
  pids[pos] = sysCreateProcess(sizeof(argvPhylo) / sizeof(argvPhylo[0]), argvPhylo, my_phylo_process);
  if (pids[pos] == -1) {
    printf("Failed creating process for philosopher %d\n", pos);
    sysDestroySemaphore(forks[pos]);
    freeAll();
    return -1;
  }
  philos_on_table++;
  freeAll();
  printf("Philosopher number %d has joined the game\n", pos + 1);

  return 0;
}

int rem_phylo(int pos) {
  if (check_phylo_pos(pos) || pos >= philos_on_table) return -1;
  blockAll();
  if (!sysDestroySemaphore(forks[pos])) {
    printf("Error destroying Philosopher %d's semaphore.\n", pos);
    freeAll();
    sysExit(PROCESS_FAILURE);
  }
  sysKill(pids[pos]);
  forks[pos] = 0;
  pids[pos] = 0;
  philos_on_table = pos;
  printf("Philosopher number %d has left the game\n", pos + 1);
  freeAll();
  return 0;
}

void end_phylos() {
  blockAll();
  while (philos_on_table > 0) {
    int pos = philos_on_table - 1;
    if (!sysDestroySemaphore(forks[pos]) || !sysKill(pids[pos])) {
      printf("Error deleting philosopher %d\n", pos + 1);
      freeAll();
      sysExit(PROCESS_FAILURE);
    }
    printf("Philosopher %d (PID: %d, Fork semaphore: %d) has left the game\n", pos + 1, pids[pos], forks[pos]);
    philos_on_table--;
  }
  if (!sysDestroySemaphore(change_philos_sem)) {
    printf("Error deleting mutex\n");
    sysExit(PROCESS_FAILURE);
  }
  printf("All philosophers have left the game.\n");
}

void commandPhylo(int argc, char* argv[argc]) {
  if (argc != 1) {
    sysExit(TOO_MANY_ARGUMENTS);
  }
  for (int i = 0; i < PHILO_MAX_AMOUNT; ++i) {
    philo_state[i] = 0;
    forks[i] = 0;
    pids[i] = 0;
    philos_on_table = 0;
    change_philos_sem = 0;
  }

  for (int i = 0; i < PHILO_AMOUNT; i++) {
    forks[i] = sysSemInit(1);
    if (forks[i] == -1) {
      printf("Error creating semaphore\n");
      sysExit(PROCESS_FAILURE);
    }
  }
  change_philos_sem = sysCreateSemaphore(PHYLO_MUTEX, PHILO_AMOUNT);
  printf("%d", change_philos_sem);
  if (change_philos_sem == -1) {
    printf("Error creating mutex\n");
    sysExit(PROCESS_FAILURE);
  }
  for (int i = 0; i < PHILO_AMOUNT; i++) {
    char philo_num[3];
    uintToBase(i, philo_num, 10);
    const char* argvPhylo[] = {"my_phylo_process", philo_num};
    pids[i] = sysCreateProcess(sizeof(argvPhylo) / sizeof(argvPhylo[0]), argvPhylo, my_phylo_process);
    if (pids[i] == -1) {
      printf("Error creating process for philosopher %d\n", i);
      sysExit(PROCESS_FAILURE);
    }
    philo_state[i] = THINKING;
    philos_on_table++;
    printf("Philosopher number %d has joined the game\n", i + 1);
  }

  KeyStruct key;
  while (getKey(&key)) {
    if (key.character == 'a' || key.character == 'A') {
      printf("Adding philosopher... %d\n", philos_on_table + 1);
      add_phylo(philos_on_table);
    } else if (key.character == 'r' || key.character == 'R') {
      printf("Removing philosopher... %d\n", philos_on_table);
      rem_phylo(philos_on_table - 1);
    } else if (key.character == 'e' || key.character == 'E') {
      end_phylos();
      printf("Ending philosopher...\n");
      sysExit(SUCCESS);
    }
  }
}
