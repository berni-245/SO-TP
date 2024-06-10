#include <shellUtils.h>
#include <utils.h>

#define PHILO_AMOUNT 5
#define PHILO_MAX_AMOUNT 10
#define PHILO_MIN_AMOUNT 2

#define PHYLO_MUTEX "phylo"
#define PHILO_NAME_LENGTH 5


int philos_on_table;
char* philo_name[PHILO_MAX_AMOUNT];
int philo_state[PHILO_AMOUNT];
int forks[PHILO_AMOUNT];
uint64_t pids[PHILO_AMOUNT];


enum phylo{
  EATING,
  HUNGRY,
  THINKING
};

int right_fork(int i){
  return (i+1)%philos_on_table;
}
int left_fork(int i){
  return (i-1)%philos_on_table;
}

void change_philo_action(int value, int pos, int mutex_sem){
  sysWaitSem(mutex_sem);
  philo_state[pos]=value;
  sysPostSem(mutex_sem);
}

void monitor(int mutex_sem){
  sysWaitSem(mutex_sem);
  for (int i = 0; i < philos_on_table; i++) {
    printf("%c ", philo_state[i]? '.':'E');
  }
  sysPostSem(mutex_sem);
  printf("\n");
}


void take_forks(int i, int mutex_sem){
  change_philo_action(HUNGRY,i, mutex_sem);
  if(i % 2){
    sysWaitSem(forks[right_fork(i)]);
    sysWaitSem(forks[i]);
  } else {
    sysWaitSem(forks[i]);
    sysWaitSem(forks[right_fork(i)]);
  }
}
void leave_forks(int i){
  sysPostSem(forks[i]);
  sysPostSem(forks[right_fork(i)]);
}
void think(int i, int mutex_sem){
  change_philo_action(THINKING,i, mutex_sem);
  sysSleep(randBetween(1000, 2000));
}
void eat(int i, int mutex_sem){
  change_philo_action(EATING,i, mutex_sem);
  sysSleep(randBetween(1000, 2000));
  monitor(mutex_sem);
}


void my_phylo_process(uint64_t argc, char* argv[argc]) {
  int n = strToInt(argv[1]);
  if (n < 0) {
    sysExit(ILLEGAL_ARGUMENT);
  }
  int mutex_sem = sysOpenSem(PHYLO_MUTEX, 1);
  if (mutex_sem < 0) {
    printf("my_process_inc: ERROR opening semaphore\n");
    sysExit(MISSING_ARGUMENTS);
  }

  while(1) {
    think(n, mutex_sem);
    take_forks(n, mutex_sem);
    eat(n, mutex_sem);
    leave_forks(n);
  }
  sysExit(SUCCESS);
}


int check_phylo_pos(int pos){
  return (pos > PHILO_MAX_AMOUNT || pos< PHILO_MIN_AMOUNT);
}


int add_Phylo(int pos){
  if (check_phylo_pos(pos))return -1;
  int mutex_sem= sysOpenSem(PHYLO_MUTEX, 1);
  sysWaitSem(mutex_sem);
  char sem_name[NAME_MAX_LEN]="ph_a";
  sem_name[3]=sem_name[3]+pos;
  forks[pos]=sysCreateSemaphore(sem_name, 1);
  if(forks[pos]==-1){
    sysExit(PROCESS_FAILURE);
  }
  philo_name[pos]= sysMalloc(MAX_NAME_LENGTH);
  strcpy(philo_name[pos], sem_name);
  philo_state[pos]=THINKING;
  char phylo_num[3];
  uintToBase(pos, phylo_num, 10);
  const char* argvPhylo[] = {"my_phylo_process", phylo_num};
  printf("Philosopher number %d has joined the game\n", pos);
  philos_on_table++;
  sysPostSem(mutex_sem);

  pids[pos] = sysCreateProcess(sizeof(argvPhylo) / sizeof(argvPhylo[0]), argvPhylo, my_phylo_process);
  return 0;
}
int rem_Phylo(int pos){
  if (check_phylo_pos(pos))return -1;
  int mutex_sem= sysOpenSem(PHYLO_MUTEX, 1);
  sysWaitSem(mutex_sem);
  --philos_on_table;
  if (philo_state[pos] == EATING){
    leave_forks(pos);
  }
  if (!sysDestroySemaphoreByName(philo_name[pos]))
    printf("Error destroying Philosopher´s semaphore.\n");
  sysFree(philo_name[pos]);
  sysKill(pids[pos]);
  sysPostSem(mutex_sem);
  return 0;
}
void end_phylos(){
  int mutex_sem= sysOpenSem(PHYLO_MUTEX, 1);
  while (philos_on_table > 0){
    sysWaitSem(mutex_sem);
    if (!sysDestroySemaphoreByName(philo_name[philos_on_table-1]) || !sysKill(pids[philos_on_table - 1])){
      printf("Error deleting phylo\n");
      sysExit(PROCESS_FAILURE);
    }
    philos_on_table--;
    sysPostSem(mutex_sem);
  }
  if (!sysDestroySemaphoreByName(PHYLO_MUTEX)){
    printf("Error deleting mutex\n");
    return;
  }
  printf("Phylo has been ended\n");
  sysExit(SUCCESS);
}
void commandPhylo(int argc, char* argv[argc]) {
  if (argc != 1) {
    sysExit(TOO_MANY_ARGUMENTS);
  }
  char sem_name[NAME_MAX_LEN]="ph_a";

  for (int i = 0; i < PHILO_AMOUNT; i++) {
    forks[i]=sysCreateSemaphore(sem_name, 1);
    if(forks[i]==-1){
      printf("Error creating semaphore: %s\n", sem_name);
      sysExit(PROCESS_FAILURE);
    }
    philo_name[i]= sysMalloc(MAX_NAME_LENGTH);
    strcpy(philo_name[i], sem_name);
    sem_name[3]=sem_name[3]+1;
  }
  int mutex_sem=sysCreateSemaphore(PHYLO_MUTEX, 1);
  sysWaitSem(mutex_sem);
  for (int i = 0; i < PHILO_AMOUNT; i++) {
    char philo_num[3];
    uintToBase(i, philo_num, 10);
    const char* argvPhylo[] = {"my_phylo_process", philo_num};
    philo_state[i]=THINKING;
    pids[i] = sysCreateProcess(sizeof(argvPhylo) / sizeof(argvPhylo[0]), argvPhylo, my_phylo_process);
    philos_on_table++;
    printf("Philosopher number %d has joined the game\n", i);
  }
  sysPostSem(mutex_sem);

  KeyStruct key;
  while (1) {
    sysHalt();
    if (getKey(&key) != EOF){
      if (key.character == 'a' || key.character=='A'){
        add_Phylo(philos_on_table);
      }
      else if (key.character == 'r' || key.character=='R'){
        rem_Phylo(philos_on_table-1);
      }
      else if (key.character == 'e' || key.character=='E'){
        end_phylos();
      }
    }
  }


}
