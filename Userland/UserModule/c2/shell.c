#include "syscalls.h"
#include <shellUtils.h>

extern uint8_t bss;

// Circular buffer. Stores just the command indexes in the global screenBuffer.
int commandHistory[MAX_HISTORY_LEN];
// Where in the commandHistory the new command's index should go.
int historyNewIdx = 0;
// Stores where in the commandHistory we are standing while traversing it with
// historyPrev and historyNext.
int historyCurrentIdx = 0;
// How many commands have been stored. Can't go back more than this.
int historyCount = 0;
// How many elements remain to go back through while traversing the commandHistory.
int historyCurrentCount = 0;

// Index in the screenBuffer of the start of the current command.
int currentCommandIdx = 0;

static int commandReturnCode = 0;

int64_t global;
#define SEM_ID "sem"
#define TOTAL_PAIR_PROCESSES 2

int shell() {
  setShellColors(0xC0CAF5, 0x1A1B26, 0xFFFF11);
  clearScreen();

  addCommand("help", "List all commands and their descriptions.", commandHelp);
  addCommand("echo", "Print all arguments.", commandEcho);
  addCommand("$?", "Print previous command return code.", commandGetReturnCode);
  addCommand("realTime", "Get current time", commandRealTime);
  addCommand("keyInfo", "Get pressed key info. Exit with ctrl+c.", commandGetKeyInfo);
  addCommand("rand", "Generate random numbers.", commandRand);
  addCommand("layout", "Get or set current layout. \n    Available flags: --help, --list", commandLayout);
  addCommand("setColors", "Set font and background colors.", commandSetColors);
  addCommand("sysInfo", "Get some system information.", commandSysInfo);
  addCommand(
      "getRegisters", "Get the values of the saved registers. \n    Available flags: --help", commandGetRegisters
  );
  addCommand("snake", "Play snake.", commandSnake);
  addCommand("zeroDivisionError", "Test the zero division error", commandZeroDivisionError);
  addCommand("invalidOpcodeError", "Test the invalid opcode error", commandInvalidOpcodeError);
  addCommand("ps", "Print process list", commandPs);
  addCommand("createSem", "Creates a semaphore", commandCreateSemaphore);
  addCommand("destroySem", "Destroys a semaphore", commandDestroySemaphore);
  addCommand("testSem", "Takes global variable to 1000", commandTestSem);

  char* argv[1] = {"help"};
  sysWaitPid(sysCreateProcess(1, argv, commandHelp));

  newPrompt();

  KeyStruct key;
  while (1) {
    sysHalt();
    if (getKey(&key) != EOF) {
      if (key.md.ctrlPressed) {
        switch (toLower(key.character)) {
        case '+':
          incFont();
          break;
        case '-':
          decFont();
          break;
        case 'l':
          clearScreen();
          screenBufWriteIdx = 0;
          screenBufReadIdx = 0;
          newPrompt();
          break;
        case 'w':
          deleteWord();
          break;
        case 'k':
          historyPrev();
          break;
        case 'j':
          historyNext();
          break;
        }
      } else {
        if (key.character == '\n') {
          if (screenBufWriteIdx != currentCommandIdx) historyPush();
          resetHistoryCurrentVals();
          printChar(key.character);
          commandReturnCode = parseCommand();
          newPrompt();
        } else if (key.character == '\b') {
          if (screenBufWriteIdx != currentCommandIdx) {
            printChar(key.character);
          }
        } else if (key.character == '\t') {
          autocomplete();
        } else {
          printChar(key.character);
        }
      }
    }
  };

  return 1;
}

void clearLine() {
  while (screenBufWriteIdx != currentCommandIdx) printChar('\b');
}

void resetHistoryCurrentVals() {
  historyCurrentIdx = historyNewIdx;
  historyCurrentCount = historyCount;
}
void historyPush() {
  commandHistory[historyNewIdx] = currentCommandIdx;
  incCircularIdx(&historyNewIdx, MAX_HISTORY_LEN);
  if (historyCount < MAX_HISTORY_LEN) ++historyCount;
}
void historyPrev() {
  if (historyCurrentCount == 0) return;
  decCircularIdx(&historyCurrentIdx, MAX_HISTORY_LEN);
  --historyCurrentCount;
  int i = commandHistory[historyCurrentIdx];
  clearLine();
  while (screenBuffer[i] != '\n') {
    printChar(screenBuffer[i]);
    incCircularIdx(&i, SCREEN_BUFFER_SIZE);
  }
}
void historyNext() {
  if (historyCurrentCount >= historyCount - 1) return;
  incCircularIdx(&historyCurrentIdx, MAX_HISTORY_LEN);
  ++historyCurrentCount;
  int i = commandHistory[historyCurrentIdx];
  clearLine();
  while (screenBuffer[i] != '\n') {
    printChar(screenBuffer[i]);
    incCircularIdx(&i, SCREEN_BUFFER_SIZE);
  }
}

int getCurrentChar() {
  return screenBuffer[(screenBufWriteIdx > 0) ? screenBufWriteIdx - 1 : SCREEN_BUFFER_SIZE - 1];
}
void deleteWord() {
  if (screenBufWriteIdx == currentCommandIdx) return;
  char* wordSeps = " -.:,;";
  if (!strContains(wordSeps, getCurrentChar())) {
    do {
      printChar('\b');
    } while (!strContains(wordSeps, getCurrentChar()));
  } else {
    do {
      printChar('\b');
    } while (strContains(wordSeps, getCurrentChar()) && (screenBufWriteIdx != currentCommandIdx));
  }
}

void setShellColors(uint32_t fontColor, uint32_t bgColor, uint32_t cursorColor) {
  setFontColor(fontColor);
  setBgColor(bgColor);
  setCursorColor(cursorColor);
}

static const char* const prompt = " > ";
static const char* const errorPrompt = " >! ";
static int currentPromptLen = 0;
void newPrompt() {
  const char* currentPrompt;
  if (commandReturnCode == 0) {
    currentPrompt = prompt;
    currentPromptLen = 3;
  } else {
    currentPrompt = errorPrompt;
    currentPromptLen = 4;
  }
  printString(currentPrompt);
  currentCommandIdx = screenBufWriteIdx;
}

void incFont() {
  setFontSize(systemInfo.fontSize + 1);
  screenBufReadIdx = currentCommandIdx - currentPromptLen;
  if (screenBufReadIdx < 0) screenBufReadIdx = SCREEN_BUFFER_SIZE + screenBufReadIdx;
  repaint();
}
void decFont() {
  setFontSize(systemInfo.fontSize - 1);
  repaint();
}

static ShellCommand commands[MAX_COMMAND_COUNT];
static int commandCount = 0;
void addCommand(char* name, char* description, ShellFunction function) {
  if (commandCount >= MAX_COMMAND_COUNT) return;
  ShellCommand newCommand = {.name = name, .description = description, .function = function};
  commands[commandCount++] = newCommand;
}
ShellFunction getCommand(char* name) {
  for (int i = 0; i < commandCount; ++i) {
    if (strcmp(name, commands[i].name) == 0) return commands[i].function;
  }
  return NULL;
}

void autocomplete() {
  int matchCount = 0, matchIdx = 0, len = 0;
  screenBuffer[screenBufWriteIdx] = 0;
  for (int i = 0; i < commandCount; ++i) {
    char* command = commands[i].name;
    bool match = true;
    int k = 0;
    for (int j = currentCommandIdx; screenBuffer[j] != 0 && command[k] != 0 && match; ++j, ++k) {
      if (screenBuffer[j] == ' ') return;
      else if (screenBuffer[j] != command[k]) match = false;
    }
    if (match && command[k] != 0) {
      ++matchCount;
      if (matchCount > 1) return;
      matchIdx = i;
      len = k;
    }
  }
  if (matchCount == 0) return;
  printString(commands[matchIdx].name + len);
}

ExitCode parseCommand() {
  // MAX_ARG_COUNT is large enough for me to ignore the error of the user going
  // over it. It's a pain to take it into account. If it has to be done I'd rather
  // make a dynamic sized array.
  char* argv[MAX_ARG_COUNT];
  int argc = 0, len = 0;
  int i = currentCommandIdx;
  incCircularIdxBy(&i, strTrimStartOffset(screenBuffer + i), SCREEN_BUFFER_SIZE);
  argv[0] = sysMalloc(MAX_ARG_LEN + 1);
  ShellFunction command;
  char c;
  do {
    c = screenBuffer[i];
    if (c == ' ') {
      argv[argc++][len] = 0;
      len = 0;
      incCircularIdxBy(&i, strTrimStartOffset(screenBuffer + i), SCREEN_BUFFER_SIZE);
      c = screenBuffer[i];
      if (c != '\n') argv[argc] = sysMalloc(MAX_ARG_LEN + 1);
    } else if (c == '\n') {
      argv[argc++][len] = 0;
      len = 0;
    } else {
      if (len >= MAX_ARG_LEN) {
        argv[argc][MAX_ARG_LEN] = 0;
        printf("%s: %s...\n", CommandResultStrings[ARGUMENT_TOO_LONG], argv[argc]);
        for (int i = 0; i < argc; ++i) sysFree(argv[i]);
        return ARGUMENT_TOO_LONG;
      }
      argv[argc][len++] = c;
      incCircularIdx(&i, SCREEN_BUFFER_SIZE);
    }
  } while (c != '\n');

  if (argv[0][0] == 0) return SUCCESS;
  // It's inefficient checking if the command is valid after parsing all the arguments.
  // But doing it right after parsing the first word was annoying because there're two
  // cases: `command arg1 ...\n` and `command\n`
  command = getCommand(argv[0]);
  if (command == NULL) {
    printf("%s: %s\n", CommandResultStrings[COMMAND_NOT_FOUND], argv[0]);
    sysFree(argv[0]);
    return COMMAND_NOT_FOUND;
  }

  if (strcmp(argv[argc - 1], "&") == 0) {
    int pid = sysCreateProcess(argc - 1, argv, command);
    for (int i = 0; i < argc; ++i) sysFree(argv[i]);
    printf("Running in background '%s', pid: %d\n", argv[0], pid);
    return SUCCESS;
  } else {
    int pid = sysCreateProcess(argc, argv, command);
    for (int i = 0; i < argc; ++i) sysFree(argv[i]);
    return sysWaitPid(pid);
  }
}

void commandEcho(int argc, char* argv[argc]) {
  // Starts at 1 because first arg is the command name
  for (int i = 1; i < argc; ++i) {
    printf("%s ", argv[i]);
  }
  printChar('\n');
  sysExit(SUCCESS);
}

void commandGetReturnCode() {
  printf("%s - Code: %d\n", CommandResultStrings[commandReturnCode], commandReturnCode);
  sysExit(SUCCESS);
}

void commandRealTime() {
  Time currentTime;
  sysGetCurrentTime(&currentTime);
  printf("%s\n", currentTime.string);
  sysExit(SUCCESS);
}

void commandHelp() {
  printString("Available commands:\n");
  for (int i = 0; i < commandCount; ++i) {
    printf("\t- %s: %s\n", commands[i].name, commands[i].description);
  }
  sysExit(0x999);
}

void commandGetKeyInfo() {
  KeyStruct key;
  while (1) {
    sysHalt();
    if (getKey(&key) != EOF) {
      if (justCtrlMod(&key) && key.character == 'c') sysExit(SUCCESS);
      else {
        printKey(&key);
      }
    }
  }
  sysExit(SUCCESS);
}

// argc aka rdi is arriving as 0 for some reason.
void commandRand(int argc, char* argv[argc]) {
  static bool randInitialized = false;
  if (!randInitialized) {
    setSrand(sysGetTicks());
    randInitialized = true;
  }
  if (argc < 3) {
    puts("Usage:");
    printf("\t\t%s <min> <max> [count]\n", argv[0]);
    printf("Where all arguments are integers and count is optional.\n");
    sysExit(MISSING_ARGUMENTS);
  }
  int min = strToInt(argv[1]);
  int max = strToInt(argv[2]);
  if (max < min) {
    printf("Error: min (%d) can't be greater than max (%d)\n", min, max);
    sysExit(ILLEGAL_ARGUMENT);
  }
  int count = (argc > 3) ? strToInt(argv[3]) : 1;
  while (count--) {
    printf("%d%s", randBetween(min, max), (count == 0) ? "" : ", ");
  }
  printf("\n");
  sysExit(SUCCESS);
}

void commandLayout(int argc, char* argv[argc]) {
  if (argc == 1) {
    printf("Current layout: %s - %d\n", LayoutStrings[systemInfo.layout], systemInfo.layout);
    sysExit(SUCCESS);
  }
  if (strcmp(argv[1], "--help") == 0) {
    printf("Usage\n");
    printf("\t\t%s [option] [layout]\n", argv[0]);
    printf("Options:\n");
    printf("\t\t--help    print this help message.\n");
    printf("\t\t--list    list all available layouts.\n");
    printf("If `layout` is not included then get current layout. If `layout` included then set system layout.\n");
    printf("`layout` should be a valid code. To see valid values for the"
           "`layout` argument use the --list option\n");
  } else if (strcmp(argv[1], "--list") == 0) {
    printf("- %s: %d\n", LayoutStrings[QWERTY_LATAM], QWERTY_LATAM);
    printf("- %s: %d\n", LayoutStrings[QWERTY_US], QWERTY_US);
  } else {
    int code = strToInt(argv[1]);
    if (code != QWERTY_LATAM && code != QWERTY_US) {
      printf("Layout not available: %s\n", argv[1]);
      sysExit(ILLEGAL_ARGUMENT);
    }
    setLayout(code);
    printf("Layout set to %s\n", LayoutStrings[code]);
  }
  sysExit(SUCCESS);
}

void commandSetColors(int argc, char* argv[argc]) {
  if (argc < 4) {
    puts("Usage:");
    printf("\t\t%s <fontColor> <backgroundColor> <cursorColor>\n", argv[0]);
    printf("Where all arguments should be hex colors.\n");
    sysExit(MISSING_ARGUMENTS);
  }
  int fontColor = strToInt(argv[1]);
  int bgColor = strToInt(argv[2]);
  int cursorColor = strToInt(argv[3]);
  setFontColor(fontColor);
  setBgColor(bgColor);
  setCursorColor(cursorColor);
  repaint();
  sysExit(SUCCESS);
}

void commandSysInfo() {
  printf("screenWidth: %d\n", systemInfo.screenWidth);
  printf("screenHeight: %d\n", systemInfo.screenHeight);
  printf("charWidth: %d\n", systemInfo.charWidth);
  printf("charHeight: %d\n", systemInfo.charHeight);
  printf("layout: %d\n", systemInfo.layout);
  printf("fontSize: %d\n", systemInfo.fontSize);
  printf("charSeparation: %d\n", systemInfo.charSeparation);
  printf("fontCols: %d\n", systemInfo.fontCols);
  printf("fontRows: %d\n", systemInfo.fontRows);
  sysExit(SUCCESS);
}

void commandGetRegisters(int argc, char* argv[argc]) {
  if (argc >= 2 && strcmp(argv[1], "--help") == 0) {
    puts("Usage:");
    printf("\t\t%s\n", argv[0]);
    printf("You can save the values of the registers at any time by pressing F1 "
           "and by running this command without this flag it will print the saved "
           "values of the registers.\n");
    sysExit(SUCCESS);
  }
  Register registers[REGISTER_QUANTITY];
  sysGetRegisters(registers);
  // Print 3 registers per row.
  for (int i = 0; i < REGISTER_QUANTITY; i++) {
    printf("%s %016lx ", registers[i].name, registers[i].value);
    if (i % 3 == 0) printf("\n");
  }
  printf("\n");
  printf("For more info add --help to the command\n");
  sysExit(SUCCESS);
}

void commandSnakeUsage(char* commandName) {
  puts("Usage:");
  printf("\t\t%s [options] <player1Name> [player2Name]\n", commandName);
  printf("Options:\n");
  printf("\t\t--mute    don't play any sounds.\n");
  printf("Player 1 moves with wasd, player 2 with ijkl. Other keybinds are:\n");
  printf(" ctrl + r: reset game\n");
  printf(" ctrl + x: lose game\n");
  printf(" ctrl + c: exit game\n");
}

void commandSnake(int argc, char* argv[argc]) {
  if (argc < 2) {
    commandSnakeUsage(argv[0]);
    sysExit(MISSING_ARGUMENTS);
  } else {
    int argIdx = 1;
    int playerCount = argc - 1;
    bool mute = false;
    if (strcmp(argv[argIdx], "--mute") == 0) {
      mute = true;
      ++argIdx;
      --playerCount;
    }
    if (playerCount < 1) {
      commandSnakeUsage(argv[0]);
      sysExit(MISSING_ARGUMENTS);
    }
    uint32_t fontColor = getFontColor();
    uint32_t bgColor = getBgColor();
    uint32_t cursorColor = getCursorColor();
    if (playerCount == 1) {
      snake(false, argv[argIdx], "", mute);
    } else {
      snake(true, argv[argIdx], argv[argIdx + 1], mute);
    }
    setShellColors(fontColor, bgColor, cursorColor);
  }
  repaint();
  sysExit(SUCCESS);
}

void commandZeroDivisionError() {
  // Always set srand because after the exception the modules starts anew
  // and srand is zero again.
  setSrand(sysGetTicks());
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiv-by-zero"
  sysExit(rand() / 0);
#pragma GCC diagnostic pop
}

void commandPs() {
  int len;
  PCB* pcbList = sysPCBList(&len);
  printf("%3s, %-10s, %-10s, %10s, %10s, %10s\n", "PID", "Name", "State", "rsp", "rbp", "Priority");
  for (int i = 0; i < len; ++i) {
    PCB* pcb = pcbList + i;
    printf("%3d, %-10s, %-10s, %p, %p, %10d\n", pcb->pid, pcb->name, pcb->state, pcb->rsp, pcb->rbp, pcb->priority);
  }
  sysFree(pcbList);
  sysExit(SUCCESS);
}

void commandCreateSemaphore(int argc, char* argv[argc]) {
    if (argc != 3) {
        puts("Usage:");
        printf("\t\t%s <semName> <semValue>\n", argv[0]);
        sysExit(MISSING_ARGUMENTS);
    }
    char* semName = argv[1];
    int semValue = strToInt(argv[2]);
    int sem_id = sysCreateSemaphore(semName, semValue);
    if (sem_id == -1) {
        puts("Error creating semaphore");
    } else {
        printf("Semaphore created with ID: %d\n", sem_id);
    }
    sysExit(SUCCESS);
}
void commandDestroySemaphore(int argc, char* argv[argc]){
    if (argc!=2){
        puts("Usage:");
        printf("\t\t%s <semName>\n", argv[0]);
        sysExit(MISSING_ARGUMENTS);
    }
    printf("%s\n", argv[1]);
    int sem=sysDestroySemaphore(argv[1]);
    printf("%5d\n", sem);
    sysExit(SUCCESS);
}
void slowInc(int64_t *p, int64_t inc) {
    uint64_t aux = *p;
    commandChangeProcess(); // This makes the race condition highly probable
    aux += inc;
    *p = aux;
}


void my_process_inc(uint64_t argc, char *argv[]) {
    //if (argc != 4) {
        sysExit(SUCCESS);
    //}

    int n = strToInt(argv[1]);
    int inc = strToInt(argv[2]);
    int use_sem = strToInt(argv[3]);

    if (n <= 0 || inc == 0 || use_sem < 0) {
        sysExit(ILLEGAL_ARGUMENT);
    }

    int sem=0;
    if (use_sem) {
        //sem = sysOpenSem("sem", 1);
        if (sem < 0) {
            printf("my_process_inc: ERROR opening semaphore\n");
            sysExit(MISSING_ARGUMENTS);
        }
    }

    for (int i = 0; i < n; i++) {
        if (use_sem) {
            sysWaitSem(sem);
        }
        slowInc(&global, inc);
        if (use_sem) {
            sysPostSem(sem);
        }
    }
    //if (use_sem)
    //    sysDestroySemaphore("sem");
    printf("Final value in process: %d\n", global);
    sysExit(SUCCESS);
}
void commandTestSem(uint64_t argc, char *argv[]) { //{n, use_sem, 0}
    if (argc!=4){
        sysExit(MISSING_ARGUMENTS);
    }

    uint64_t pids[2 * TOTAL_PAIR_PROCESSES];

    char *argvDec[] = {"my_process_inc", argv[1], "-1", argv[2], NULL};
    char *argvInc[] = {"my_process_inc", argv[1], "1", argv[2], NULL};

    global = 0;
    int sem = sysCreateSemaphore("sem", 1);
    uint64_t i;
    for (i = 0; i < TOTAL_PAIR_PROCESSES; i++) {
        pids[i] = sysCreateProcess(4, argvDec, my_process_inc);
        pids[i + TOTAL_PAIR_PROCESSES] = sysCreateProcess(4, argvInc, my_process_inc);
    }


    for (i = 0; i < TOTAL_PAIR_PROCESSES; i++) {
        sysWaitPid(pids[i]);
        sysWaitPid(pids[i + TOTAL_PAIR_PROCESSES]);
    }

    printf("Final value: %d\n", global);
    //sysDestroySemaphore("sem");
    sysExit(SUCCESS);
}


