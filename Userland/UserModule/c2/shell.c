#include <array.h>
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
static int currentPromptLen = 0;

Array currentCommand;

int shell() {
  setShellColors(0xC0CAF5, 0x1A1B26, 0xFFFF11);
  clearScreen();

  currentCommand = Array_initialize(sizeof(char), 100, NULL, NULL);

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
  addCommand("testSem", "Test semaphores by using multiple processes to modifying shared variable", commandTestSem);

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
          clearScreenKeepCommand();
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
          Array_clear(currentCommand);
        } else if (key.character == '\b') {
          if (screenBufWriteIdx != currentCommandIdx) {
            printChar(key.character);
            Array_pop(currentCommand);
          }
        } else if (key.character == '\t') {
          autocomplete();
        } else {
          printChar(key.character);
          Array_push(currentCommand, &key.character);
        }
      }
    }
  }

  return 1;
}

void clearLine() {
  while (screenBufWriteIdx != currentCommandIdx) printChar('\b');
  Array_clear(currentCommand);
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
void historyCopy() {
  int i = commandHistory[historyCurrentIdx];
  clearLine();
  while (screenBuffer[i] != '\n') {
    printChar(screenBuffer[i]);
    Array_push(currentCommand, screenBuffer + i);
    incCircularIdx(&i, SCREEN_BUFFER_SIZE);
  }
}
void historyPrev() {
  if (historyCurrentCount == 0) return;
  decCircularIdx(&historyCurrentIdx, MAX_HISTORY_LEN);
  --historyCurrentCount;
  historyCopy();
}
void historyNext() {
  if (historyCurrentCount >= historyCount - 1) return;
  incCircularIdx(&historyCurrentIdx, MAX_HISTORY_LEN);
  ++historyCurrentCount;
  historyCopy();
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

void clearScreenKeepCommand() {
  int currentIdxLocal = currentCommandIdx;
  decCircularIdxBy(&currentIdxLocal, currentPromptLen, SCREEN_BUFFER_SIZE);
  screenBufReadIdx = currentIdxLocal;
  repaint();
}

void incFont() {
  setFontSize(systemInfo.fontSize + 1);
  clearScreenKeepCommand();
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
    // Needs to be updated to use currentCommand array.
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

ShellFunction verifyCommand(Array argv) {
  Array arg = *(Array*)Array_get(argv, 0);
  const char* argv0 = Array_getVanillaArray(arg);
  ShellFunction command = getCommand(argv0);
  if (command == NULL) {
    printf("%s: %s\n", CommandResultStrings[COMMAND_NOT_FOUND], argv0);
    Array_free(argv);
    return NULL;
  }
  return command;
}

void freeArrayPtr(Array* ele) {
  Array_free(*ele);
}

ExitCode parseCommand() {
  Array argv = Array_initialize(sizeof(Array), 10, NULL, (FreeEleFn)freeArrayPtr);
  ShellFunction command;
  Array arg = NULL;
  const char* cc = Array_getVanillaArray(currentCommand);
  int commandLength = Array_getLen(currentCommand);
  bool newWord = true;
  for (int i = 0; i < commandLength; ++i) {
    if (cc[i] == ' ') newWord = true;
    else {
      if (newWord) {
        if (arg != NULL) {
          char end = 0;
          Array_push(arg, &end);
        }
        // Added this check here too so we can exit immediately if the first argument
        // parsed isn't a valid command.
        if (Array_getLen(argv) == 1) {
          command = verifyCommand(argv);
          if (!command) return COMMAND_NOT_FOUND;
        }
        arg = Array_initialize(sizeof(char), 30, NULL, NULL);
        Array_push(argv, &arg);
        newWord = false;
      }
      Array_push(arg, cc + i);
    }
  }

  int argc = Array_getLen(argv);
  if (argc == 0) return SUCCESS;
  else if (argc == 1) {
    command = verifyCommand(argv);
    if (!command) return COMMAND_NOT_FOUND;
  }

  const char** realArgv = sysMalloc(sizeof(char*) * argc);
  for (int i = 0; i < argc; ++i) {
    realArgv[i] = Array_getVanillaArray(*(Array*)Array_get(argv, i));
  }
  if (strcmp(realArgv[argc - 1], "&") == 0) {
    int pid = sysCreateProcess(Array_getLen(argv) - 1, realArgv, command);
    Array_free(argv);
    sysFree(realArgv);
    printf("Running in background '%s', pid: %d\n", realArgv[0], pid);
    return SUCCESS;
  } else {
    int pid = sysCreateProcess(argc, realArgv, command);
    Array_free(argv);
    sysFree(realArgv);
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
    printf("Where all arguments should be numeric representations of rgb colors (can use '0x' prefix for hexa).\n");
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
    sysExit(SUCCESS);
  }

  int n = strToInt(argv[1]);
  int inc = strToInt(argv[2]);
  int use_sem = strToInt(argv[3]);

  if (n <= 0 || inc == 0 || use_sem < 0) {
    sysExit(ILLEGAL_ARGUMENT);
  }

  if (use_sem) {
    int sem = sysOpenSem("sem", 1);
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
  if (use_sem) sysDestroySemaphore("sem");
  printf("Final value in process: %l\n", globalForSemTest);
  sysExit(SUCCESS);
}

void commandTestSem(int argc, char* argv[argc]) {
  if (argc != 3) {
    printf("Usage: %s <n> <use_sem>\n", argv[0]);
    printf("\tn: number of iterations for each process\n");
    printf("\tuse_sem: 0 for no semaphores, not 0 to use semaphores\n");
    sysExit(MISSING_ARGUMENTS);
  }

  uint64_t pids[2 * TOTAL_PAIR_PROCESSES];

  char* argvDec[] = {"my_process_dec", argv[1], "-1", argv[2]};
  char* argvInc[] = {"my_process_inc", argv[1], "1", argv[2]};

  globalForSemTest = 0;
  int sem = sysCreateSemaphore("sem", 1);
  for (int i = 0; i < TOTAL_PAIR_PROCESSES; i++) {
    pids[i] = sysCreateProcess(sizeof(argvDec) / sizeof(argvDec[0]), argvDec, my_process_inc);
    pids[i + TOTAL_PAIR_PROCESSES] = sysCreateProcess(sizeof(argvDec) / sizeof(argvDec[0]), argvInc, my_process_inc);
  }

  for (int i = 0; i < TOTAL_PAIR_PROCESSES; i++) {
    sysWaitPid(pids[i]);
    sysWaitPid(pids[i + TOTAL_PAIR_PROCESSES]);
  }

  printf("Final value: %l\n", globalForSemTest);
  sysDestroySemaphore("sem");
  sysExit(SUCCESS);
}
