#include "syscalls.h"
#include <array.h>
#include <circularHistoryBuffer.h>
#include <shellUtils.h>

extern uint8_t bss;

CircularHistoryBuffer commandHistory;
int32_t currentCommandIdx = 0;

int32_t commandReturnCode = 0;
static int32_t currentPromptLen = 0;

// int commandCount = 0;
Array currentCommand;

Array commands;

void freeArrayPtr(Array* ele) {
  Array_free(*ele);
}

int32_t compareArgv(Array* argv1, Array* argv2) {
  int32_t argc = Array_getLen(*argv1);
  int32_t cmp = argc - Array_getLen(*argv2);
  if (cmp == 0) cmp = Array_equals(*argv1, *argv2) == 0;
  return cmp;
}

int32_t shell() {
  setShellColors(0xC0CAF5, 0x1A1B26, 0xFFFF11);
  clearScreen();

  currentCommand = Array_initialize(sizeof(char), 100, NULL, NULL);
  commandHistory = CHB_initialize(sizeof(Array), MAX_HISTORY_LEN, (FreeEleFn)freeArrayPtr, (CompareEleFn)compareArgv);
  commands = Array_initialize(sizeof(ShellCommand), 100, NULL, NULL);

  addCommand("superSecret", "???", commandSuperSecret);
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
      "getRegisters",
      "Get the values of the saved registers.\n"
      "    Available flags: --help",
      commandGetRegisters
  );
  addCommand("snake", "Play snake.", commandSnake);
  addCommand(
      "mem", "View the state of the memory for the whole system or\n"
      "    process-wise by specifying pid",
      commandGetMemoryState
  );
  addCommand("zeroDivisionError", "Test the zero division error.", commandZeroDivisionError);
  addCommand("invalidOpcodeError", "Test the invalid opcode error.", commandInvalidOpcodeError);
  addCommand("ps", "Print process list.", commandPs);
  addCommand("testSem", "Test semaphores by using multiple processes to\n"
      "    modifying shared variable.", commandTestSem);
  addCommand("testMM", "Test Memory manager.", commandTestMM);
  addCommand("kill", "Kill process by pid.", commandKill);
  addCommand("nice", "Change priority of a process by pid", commandNice);
  addCommand("loop", "Sends a message with the PID every given seconds", commandLoop);
  addCommand("pipeTest", "Test pipes with reader and writer processes", commandTestPipes);
  addCommand(
      "phylo",
      "Starts the philosophers problem.\n"
      "    `e` to Exit, `a` to Add philosopher, `r` to Remove philosopher",
      commandPhylo
  );
  addCommand("destroyPipe", "Destroy pipe by id", commandDestroyPipe);
  addCommand("block", "Blocks the process with the pid given", commandBlock);
  addCommand("unblock", "Unblocks the process with the pid given", commandUnBlock);
  addCommand("testPriority", "Checks the priority functionality. 0: small wait.\n" 
      "    1: long wait", commandTestPriority);
  addCommand(
      "testProcesses", "Checks the process creation, blocking,\n" 
      "    unblocking and destruction", commandTestProcesses
  );
  addCommand("cat", "Read from stdin and output to stdout", commandCat);
  addCommand("wc", "Cound words from stdin", commandWordCount);
  addCommand("filter", "Return the given array without vocals", commandFilterVocals);

  const char* argv[1] = {"help"};
  sysWaitPid(sysCreateProcess(1, argv, commandHelp));

  newPrompt();

  KeyStruct key;
  while (true) {
    getKey(&key);
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
        printChar(key.character);
        commandReturnCode = parseCommand();
        newPrompt();
        Array_clear(currentCommand);
      } else if (key.character == '\b') {
        if (Array_getLen(currentCommand) > 0) {
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

  return 1;
}

void clearLine() {
  int32_t i = Array_getLen(currentCommand);
  while (i--) printChar('\b');
  Array_clear(currentCommand);
}

void historyCopy(Array argv) {
  clearLine();
  int32_t argc = Array_getLen(argv);
  for (int32_t i = 0; i < argc; ++i) {
    const char* arg = Array_getVanillaArray(*(Array*)Array_get(argv, i));
    for (int32_t i = 0; arg[i] != 0; ++i) {
      printChar(arg[i]);
      Array_push(currentCommand, arg + i);
    }
    if (i < argc - 1) {
      char c = ' ';
      printChar(c);
      Array_push(currentCommand, &c);
    }
  }
}
void historyPrev() {
  Array* argv = CHB_readPrev(commandHistory);
  if (argv == NULL) return;
  historyCopy(*argv);
}
void historyNext() {
  Array* argv = CHB_readNext(commandHistory);
  if (argv == NULL) {
    clearLine();
    CHB_readRest(commandHistory);
    return;
  }
  historyCopy(*argv);
}

int32_t getCurrentChar() {
  char* c = Array_get(currentCommand, -1);
  if (c == NULL) return 0;
  return *c;
}
void deleteWord() {
  char* wordSeps = " -.:,;";
  char c = getCurrentChar();
  if (c == 0) return;
  if (!strContains(wordSeps, c)) {
    do {
      printChar('\b');
      Array_pop(currentCommand);
    } while ((c = getCurrentChar()) != 0 && !strContains(wordSeps, c));
  } else {
    do {
      printChar('\b');
      Array_pop(currentCommand);
    } while ((c = getCurrentChar()) != 0 && strContains(wordSeps, c));
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
}

void clearScreenKeepCommand() {
  clearScreen();
  newPrompt();
  const char* cc = Array_getVanillaArray(currentCommand);
  int32_t len = Array_getLen(currentCommand);
  for (int32_t i = 0; i < len; ++i) {
    printChar(cc[i]);
  }
}

void incFont() {
  setFontSize(systemInfo.fontSize + 1);
  clearScreenKeepCommand();
}
void decFont() {
  setFontSize(systemInfo.fontSize - 1);
  clearScreenKeepCommand();
}

void addCommand(char* name, char* description, ShellFunction function) {
  ShellCommand newCommand = {.name = name, .description = description, .function = function};
  Array_push(commands, &newCommand);
}
ShellFunction getCommand(const char* name) {
  for (int32_t i = 0; i < Array_getLen(commands); ++i) {
    ShellCommand* command = Array_get(commands, i);
    if (strcmp(name, command->name) == 0) return command->function;
  }
  return NULL;
}

void autocomplete() {
  int32_t matchCount = 0, matchIdx = 0, len = 0;
  const char* cc = Array_getVanillaArray(currentCommand);
  int32_t ccLen = Array_getLen(currentCommand);
  for (int32_t i = 0; i < Array_getLen(commands); ++i) {
    char* command = ((ShellCommand*)Array_get(commands, i))->name;
    bool match = true;
    int32_t k = 0;
    for (int32_t j = 0; j < ccLen && command[k] != 0 && match; ++j, ++k) {
      if (cc[j] == ' ') return;
      else if (cc[j] != command[k]) match = false;
    }
    if (match && command[k] != 0) {
      ++matchCount;
      if (matchCount > 1) return;
      matchIdx = i;
      len = k;
    }
  }
  if (matchCount == 0) return;
  char* match = ((ShellCommand*)Array_get(commands, matchIdx))->name;
  for (int32_t i = len; match[i] != 0; ++i) {
    printChar(match[i]);
    Array_push(currentCommand, match + i);
  }
}

ShellFunction verifyCommand(Array argv) {
  Array arg = *(Array*)Array_get(argv, 0);
  const char* argv0 = Array_getVanillaArray(arg);
  ShellFunction command = getCommand(argv0);
  if (command == NULL) {
    printf("%s: %s\n", CommandResultStrings[COMMAND_NOT_FOUND], argv0);
    // argv shouldn't be freed because I'm saving invalid commands to history too.
    // Array_free(argv);
    return NULL;
  }
  return command;
}

void setArgsNullTerminaor(Array argv) {
  int32_t argc = Array_getLen(argv);
  for (int32_t i = 0; i < argc; ++i) {
    char end = 0;
    Array_push(*(Array*)Array_get(argv, i), &end);
  }
}

void setRealArgv(int32_t argc, const char* realArgv[argc], Array argv) {
  for (int32_t i = 0; i < argc; ++i) {
    realArgv[i] = Array_getVanillaArray(*(Array*)Array_get(argv, i));
  }
}

int32_t compareArgs(Array* arg1, Array* arg2) {
  int32_t len = Array_getLen(*arg1);
  int32_t cmp = len - Array_getLen(*arg2);
  if (cmp == 0) {
    cmp = strcmp(Array_getVanillaArray(*arg1), Array_getVanillaArray(*arg2));
  }
  return cmp;
}

ExitCode parseCommand() {
  Array argv = Array_initialize(sizeof(Array), 10, (FreeEleFn)freeArrayPtr, (CompareEleFn)compareArgs);
  Array argv2 = NULL;
  Array currentArgv = argv;
  ShellFunction command;
  ShellFunction command2 = NULL;
  Array arg = NULL;
  const char* cc = Array_getVanillaArray(currentCommand);
  int32_t commandLength = Array_getLen(currentCommand);
  bool newWord = true;
  for (int32_t i = 0; i < commandLength; ++i) {
    if (cc[i] == ' ') newWord = true;
    else {
      if (newWord) {
        if (arg != NULL) {
          if (Array_getLen(arg) == 1 && *(char*)Array_get(arg, 0) == '!') {
            // No freeEleFn because it will get concatenated with argv in the end.
            argv2 = Array_initialize(sizeof(Array), 10, NULL, NULL);
            currentArgv = argv2;
          }
        }
        arg = Array_initialize(sizeof(char), 30, NULL, NULL);
        Array_push(currentArgv, &arg);
        newWord = false;
      }
      Array_push(arg, cc + i);
    }
  }

  int32_t argc = Array_getLen(argv);
  if (argc == 0) {
    Array_free(argv);
    return SUCCESS;
  }

  int32_t ret = SUCCESS;
  if (argv2 != NULL) {
    int32_t argc2 = Array_getLen(argv2);
    command2 = verifyCommand(argv2);
    if (command2 == NULL) ret = COMMAND_NOT_FOUND;
    else {
      command = verifyCommand(argv);
      if (command != NULL) {
        setArgsNullTerminaor(argv);
        setArgsNullTerminaor(argv2);
        // Dont take the pipe into account. Can't pop it because I still need it for
        // the commandHistory.
        const char* realArgv[--argc];
        const char* realArgv2[argc2];
        setRealArgv(argc, realArgv, argv);
        setRealArgv(argc2, realArgv2, argv2);
        int32_t pipe = sysPipeInit();
        ProcessPipes pipes = {.write = pipe, .read = stdin, .err = stderr};
        int32_t pid = sysCreateProcessWithPipeSwap(argc, realArgv, command, pipes);
        pipes.write = stdout;
        pipes.read = pipe;
        int32_t pid2 = sysCreateProcessWithPipeSwap(argc2, realArgv2, command2, pipes);
        sysWaitPid(pid);
        char eof = EOF;
        sysWrite(pipe, &eof, 1);
        ret = sysWaitPid(pid2);
        sysDestroyPipe(pipe);
      } else ret = COMMAND_NOT_FOUND;
    }
    Array_concat(argv, argv2);
    Array_free(argv2);
  } else {
    // I need this before verifyCommand because that function uses argv0
    // for error message if command is not found.
    setArgsNullTerminaor(argv);
    command = verifyCommand(argv);
    if (command != NULL) {
      const char* realArgv[argc];
      for (int32_t i = 0; i < argc; ++i) {
        realArgv[i] = Array_getVanillaArray(*(Array*)Array_get(argv, i));
      }
      if (strcmp(realArgv[argc - 1], "&") == 0) {
        int32_t pid = sysCreateProcess(argc - 1, realArgv, command);
        printf("Running in background '%s', pid: %d\n", realArgv[0], pid);
        ret = SUCCESS;
      } else {
        int32_t pid = sysCreateProcess(argc, realArgv, command);
        ret = sysWaitPid(pid);
      }
    } else ret = COMMAND_NOT_FOUND;
  }
  CHB_moveToFrontOrPush(commandHistory, &argv);
  return ret;
}
