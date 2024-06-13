#include <shellUtils.h>
#include <utils.h>

#define PHILO_AMOUNT 5
#define PHILO_MAX_AMOUNT 10
#define PHILO_MIN_AMOUNT 2

typedef struct philosopher{
  int state;
  int fork_at_index;
  unsigned int pid;
}philosopher;

int philos_on_table;
int change_philos_sem;
int print_mutex;
philosopher philo[PHILO_MAX_AMOUNT];
int add_rem_value;

enum phylo { EATING, HUNGRY, THINKING, NOTHING };

int right_fork(int i) {
  return (i + 1) % philos_on_table;
}

void blockAll() {
  for (int i = 0; i < philos_on_table; i++) {
    sysWaitSem(change_philos_sem);
  }
}
void blockAllToInitiate(){
  for (int i = 0; i < PHILO_AMOUNT; i++) {
    sysWaitSem(change_philos_sem);
  }
}

void freeAll(){
  for (int i = 0; i < philos_on_table; i++) {
    sysPostSem(change_philos_sem);
  }
}

void freeAllRemaining() {
  for (int i = 0; i < philos_on_table; i++) {
    while (sysPostSem(change_philos_sem));
  }
}

void monitor() {
  sysWaitSem(print_mutex);
  for (int i = 0; i < philos_on_table; i++) {
    printf("%c ", philo[i].state ? '.' : 'E');
  }
  printf("\n");
  sysPostSem(print_mutex);
}

void takeForks(int i) {
  philo[i].state = HUNGRY;
  if (i % 2) {
    sysWaitSem(philo[right_fork(i)].fork_at_index);
    sysWaitSem(philo[i].fork_at_index);
  } else {
    sysWaitSem(philo[i].fork_at_index);
    sysWaitSem(philo[right_fork(i)].fork_at_index);
  }
}

void leaveForks(int i) {
  sysPostSem(philo[i].fork_at_index);
  sysPostSem(philo[right_fork(i)].fork_at_index);
  philo[i].state = NOTHING;
}

void think(int i) {
  philo[i].state = THINKING;
  sysSleep(randBetween(600, 800));
}
void eat(int i) {
  philo[i].state = EATING;
  sysSleep(randBetween(600, 800));
  monitor();
}

void myPhyloProcess(uint64_t argc, char* argv[argc]) {
  int n = strToInt(argv[1]);
  if (n < 0) {
    sysExit(ILLEGAL_ARGUMENT);
  }
  while (1) {
    sysWaitSem(change_philos_sem);
    think(n);
    takeForks(n);
    eat(n);
    leaveForks(n);
    sysPostSem(change_philos_sem);
  }
}

int addPhylo(int pos) {
  if (pos < PHILO_MIN_AMOUNT || pos >= PHILO_MAX_AMOUNT) return -1;
  blockAll();
  philo[pos].fork_at_index = sysSemInit(1);
  if (philo[pos].fork_at_index == -1) {
    printf("Failed creating semaphore\n");
    freeAll();
    return -1;
  }
  philo[pos].state = THINKING;
  char phylo_num[3];
  uintToBase(pos, phylo_num, 10);
  const char* argvPhylo[] = {"myPhyloProcess", phylo_num};
  philo[pos].pid = sysCreateProcess(sizeof(argvPhylo) / sizeof(argvPhylo[0]), argvPhylo, myPhyloProcess);
  philos_on_table++;
  freeAll();
  printf("Philosopher number %d has joined the table\n", pos + 1);
  return 0;
}

int remPhylo(int pos) {
  if (pos < PHILO_MIN_AMOUNT || pos >= philos_on_table) return -2;
  blockAll();
  if (!sysKill(philo[pos].pid)){
    printf("Error killing Philosopher %d's process.\n", pos);
    freeAll();
    return -1;
  }
  if (!sysDestroySemaphore(philo[pos].fork_at_index)) {
    printf("Error destroying Philosopher %d's semaphore.\n", pos);
    freeAll();
    return -1;
  }
  philos_on_table = pos;
  printf("Philosopher number %d has left the table\n", pos + 1);
  freeAllRemaining();
  return 0;
}

void endPhylos() {
  blockAll();
  while (philos_on_table > 0) {
    int pos = philos_on_table - 1;
    if (!sysDestroySemaphore(philo[pos].fork_at_index) || !sysKill(philo[pos].pid)) {
      printf("Error deleting philosopher %d\n", pos + 1);
      freeAll();
      sysExit(PROCESS_FAILURE);
    }
    printf("Philosopher %d  has left the table\n", pos + 1);
    philos_on_table--;
  }
  if (!sysDestroySemaphore(change_philos_sem) || !sysDestroySemaphore(print_mutex)) {
    printf("Error deleting mutex\n");
    sysExit(PROCESS_FAILURE);
  }
  printf("All philosophers have left the table.\n");
}

void commandPhylo(int argc, char* argv[argc]) {
  if (argc != 1) {
    sysExit(TOO_MANY_ARGUMENTS);
  }
  philos_on_table = 0;
  change_philos_sem = sysSemInit(5);
  print_mutex = sysSemInit(1);
  if (change_philos_sem == -1 || print_mutex == -1) {
    printf("Error creating mutex\n");
    sysExit(PROCESS_FAILURE);
  }
  blockAllToInitiate();
  for (int i = 0; i < PHILO_AMOUNT; i++) {
    philo[i].fork_at_index = sysSemInit(1);
    if (philo[i].fork_at_index  == -1) {
      printf("Error creating semaphore\n");
      sysExit(PROCESS_FAILURE);
    }
    char philo_num[3];
    uintToBase(i, philo_num, 10);
    const char* argvPhylo[] = {"myPhyloProcess", philo_num};
    philo[i].pid = sysCreateProcess(sizeof(argvPhylo) / sizeof(argvPhylo[0]), argvPhylo, myPhyloProcess);
    philo[i].state = THINKING;
    philos_on_table++;
    printf("Philosopher number %d has joined the table\n", i + 1);
  }
  freeAll();

  KeyStruct key;
  while (getKey(&key)) {
    if (key.character == 'a' || key.character == 'A') {
      printf("Adding philosopher %d... \n", philos_on_table + 1);
      if ((add_rem_value = addPhylo(philos_on_table)) == -1)
        sysExit(PROCESS_FAILURE);
      else if (add_rem_value == -2){
        printf("Can't add more philosophers\n");
      }
    } else if (key.character == 'r' || key.character == 'R') {
      printf("Removing philosopher %d... \n", philos_on_table);
      if((add_rem_value = remPhylo(philos_on_table - 1))==-1)
        sysExit(PROCESS_FAILURE);
      else if (add_rem_value == -2){
        printf("Can't remove more philosophers\n");
      }
    } else if (key.character == 'e' || key.character == 'E') {
      printf("Ending philosophers...\n");
      endPhylos();
      sysExit(SUCCESS);
    }
  }
}
