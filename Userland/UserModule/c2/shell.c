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

int commandReturnCode = 0;
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

  const char* argv[1] = {"help"};
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

ShellCommand commands[MAX_COMMAND_COUNT];
int commandCount = 0;
void addCommand(char* name, char* description, ShellFunction function) {
  if (commandCount >= MAX_COMMAND_COUNT) return;
  ShellCommand newCommand = {.name = name, .description = description, .function = function};
  commands[commandCount++] = newCommand;
}
ShellFunction getCommand(const char* name) {
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

  const char* realArgv[argc];
  for (int i = 0; i < argc; ++i) {
    realArgv[i] = Array_getVanillaArray(*(Array*)Array_get(argv, i));
  }
  if (strcmp(realArgv[argc - 1], "&") == 0) {
    int pid = sysCreateProcess(Array_getLen(argv) - 1, realArgv, command);
    Array_free(argv);
    printf("Running in background '%s', pid: %d\n", realArgv[0], pid);
    return SUCCESS;
  } else {
    int pid = sysCreateProcess(argc, realArgv, command);
    Array_free(argv);
    return sysWaitPid(pid);
  }
}
