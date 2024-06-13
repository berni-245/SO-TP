#include "syscalls.h"
#include <array.h>
#include <circularHistoryBuffer.h>
#include <shellUtils.h>

extern uint8_t bss;

CircularHistoryBuffer commandHistory;
int currentCommandIdx = 0;

int commandReturnCode = 0;
static int currentPromptLen = 0;

// int commandCount = 0;
Array currentCommand;

Array commands;

void freeArrayPtr(Array* ele) {
  arrayFree(*ele);
}

int shell() {
  setShellColors(0xC0CAF5, 0x1A1B26, 0xFFFF11);
  clearScreen();

  currentCommand = Array_initialize(sizeof(char), 100, NULL, NULL);
  commandHistory = CHB_initialize(sizeof(Array), MAX_HISTORY_LEN, (FreeEleFn)freeArrayPtr, NULL);
  commands = Array_initialize(sizeof(ShellCommand), 100, NULL, NULL);

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
      "mem", "View the state of the memory for the whole system or process-wise by specifying pid",
      commandGetMemoryState
  );
  addCommand("zeroDivisionError", "Test the zero division error.", commandZeroDivisionError);
  addCommand("invalidOpcodeError", "Test the invalid opcode error.", commandInvalidOpcodeError);
  addCommand("ps", "Print process list.", commandPs);
  addCommand("testSem", "Test semaphores by using multiple processes to modifying shared variable.", commandTestSem);
  addCommand("testMM", "Test Memory manager.", commandTestMM);
  addCommand("kill", "Kill process by pid.", commandKill);
  addCommand("nice", "Change priority of a process by pid", commandNice);
  addCommand("getpid", "Print pid for current process.", commandGetPid);
  addCommand("loop", "Sends a message with the PID every given seconds", commandLoop);
  addCommand("pipeTest", "Test pipes with reader and writer processes", commandTestPipes);
  addCommand(
      "phylo",
      "Starts the philosophers problem, "
      "exit with e, a to add philosopher and remove with r",
      commandPhylo
  );
  addCommand("destroyPipe", "Destroy pipe by id", commandDestroyPipe);
  addCommand("block", "Blocks the process with the pid given", commandBlock);
  addCommand("unblock", "Unblocks the process with the pid given", commandUnBlock);
  addCommand("testPriority", "Checks the priority functionality. 0: small wait. 1: long wait", commandTestPriority);
  addCommand(
      "testProcesses", "Checks the process creation, blocking, unblocking and destruction", commandTestProcesses
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
        arrayClear(currentCommand);
      } else if (key.character == '\b') {
        if (arrayGetLen(currentCommand) > 0) {
          printChar(key.character);
          arrayPop(currentCommand);
        }
      } else if (key.character == '\t') {
        autocomplete();
      } else {
        printChar(key.character);
        arrayPush(currentCommand, &key.character);
      }
    }
  }

  return 1;
}

void clearLine() {
  int i = arrayGetLen(currentCommand);
  while (i--) printChar('\b');
  arrayClear(currentCommand);
}

void historyCopy(Array argv) {
  clearLine();
  int argc = arrayGetLen(argv);
  for (int i = 0; i < argc; ++i) {
    const char* arg = arrayGetVanillaArray(*(Array*)arrayGet(argv, i));
    for (int i = 0; arg[i] != 0; ++i) {
      printChar(arg[i]);
      arrayPush(currentCommand, arg + i);
    }
    if (i < argc - 1) {
      char c = ' ';
      printChar(c);
      arrayPush(currentCommand, &c);
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
    // clearLine();
    return;
  }
  historyCopy(*argv);
}

int getCurrentChar() {
  char* c = arrayGet(currentCommand, -1);
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
      arrayPop(currentCommand);
    } while ((c = getCurrentChar()) != 0 && !strContains(wordSeps, c));
  } else {
    do {
      printChar('\b');
      arrayPop(currentCommand);
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
  const char* cc = arrayGetVanillaArray(currentCommand);
  int len = arrayGetLen(currentCommand);
  for (int i = 0; i < len; ++i) {
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
  arrayPush(commands, &newCommand);
}
ShellFunction getCommand(const char* name) {
  for (int i = 0; i < arrayGetLen(commands); ++i) {
    ShellCommand* command = arrayGet(commands, i);
    if (strcmp(name, command->name) == 0) return command->function;
  }
  return NULL;
}

void autocomplete() {
  int matchCount = 0, matchIdx = 0, len = 0;
  const char* cc = arrayGetVanillaArray(currentCommand);
  int ccLen = arrayGetLen(currentCommand);
  for (int i = 0; i < arrayGetLen(commands); ++i) {
    char* command = ((ShellCommand*)arrayGet(commands, i))->name;
    bool match = true;
    int k = 0;
    // Needs to be updated to use currentCommand array.
    for (int j = 0; j < ccLen && command[k] != 0 && match; ++j, ++k) {
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
  char* match = ((ShellCommand*)arrayGet(commands, matchIdx))->name;
  for (int i = len; match[i] != 0; ++i) {
    printChar(match[i]);
    arrayPush(currentCommand, match + i);
  }
}

ShellFunction verifyCommand(Array argv) {
  Array arg = *(Array*)arrayGet(argv, 0);
  const char* argv0 = arrayGetVanillaArray(arg);
  ShellFunction command = getCommand(argv0);
  if (command == NULL) {
    printf("%s: %s\n", CommandResultStrings[COMMAND_NOT_FOUND], argv0);
    // argv shouldn't be freed because I'm saving invalid commands to history too.
    // arrayFree(argv);
    return NULL;
  }
  return command;
}

void setArgsNullTerminaor(Array argv) {
  int argc = arrayGetLen(argv);
  if (argc == 0) return;
  for (int i = 0; i < argc; ++i) {
    char end = 0;
    arrayPush(*(Array*)arrayGet(argv, i), &end);
  }
}

void setRealArgv(int argc, const char* realArgv[argc], Array argv) {
  for (int i = 0; i < argc; ++i) {
    realArgv[i] = arrayGetVanillaArray(*(Array*)arrayGet(argv, i));
  }
}

ExitCode parseCommand() {
  Array argv = Array_initialize(sizeof(Array), 10, (FreeEleFn)freeArrayPtr, NULL);
  Array argv2 = NULL;
  Array currentArgv = argv;
  ShellFunction command;
  ShellFunction command2 = NULL;
  Array arg = NULL;
  const char* cc = arrayGetVanillaArray(currentCommand);
  int commandLength = arrayGetLen(currentCommand);
  bool newWord = true;
  for (int i = 0; i < commandLength; ++i) {
    if (cc[i] == ' ') newWord = true;
    else {
      if (newWord) {
        if (arg != NULL) {
          if (arrayGetLen(arg) == 1 && *(char*)arrayGet(arg, 0) == '!') {
            // No freeEleFn because it will get concatenated with argv in the end.
            argv2 = Array_initialize(sizeof(Array), 10, NULL, NULL);
            currentArgv = argv2;
          }
        }
        arg = Array_initialize(sizeof(char), 30, NULL, NULL);
        arrayPush(currentArgv, &arg);
        newWord = false;
      }
      arrayPush(arg, cc + i);
    }
  }

  int argc = arrayGetLen(argv);
  if (argc == 0) return SUCCESS;

  int ret = SUCCESS;
  if (argv2 != NULL) {
    int argc2 = arrayGetLen(argv2);
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
        int pipe = sysPipeInit();
        ProcessPipes pipes = {.write = pipe, .read = stdin, .err = stderr};
        int pid = sysCreateProcessWithPipeSwap(argc, realArgv, command, pipes);
        pipes.write = stdout;
        pipes.read = pipe;
        int pid2 = sysCreateProcessWithPipeSwap(argc2, realArgv2, command2, pipes);
        sysWaitPid(pid);
        char eof = EOF;
        sysWrite(pipe, &eof, 1);
        ret = sysWaitPid(pid2);
        sysDestroyPipe(pipe);
      } else ret = COMMAND_NOT_FOUND;
    }
    arrayConcat(argv, argv2);
    arrayFree(argv2);
  } else {
    command = verifyCommand(argv);
    if (command != NULL) {
      setArgsNullTerminaor(argv);
      const char* realArgv[argc];
      for (int i = 0; i < argc; ++i) {
        realArgv[i] = arrayGetVanillaArray(*(Array*)arrayGet(argv, i));
      }
      if (strcmp(realArgv[argc - 1], "&") == 0) {
        int pid = sysCreateProcess(argc - 1, realArgv, command);
        printf("Running in background '%s', pid: %d\n", realArgv[0], pid);
        ret = SUCCESS;
      } else {
        int pid = sysCreateProcess(argc, realArgv, command);
        ret = sysWaitPid(pid);
      }
    } else ret = COMMAND_NOT_FOUND;
  }
  CHB_push(commandHistory, &argv);
  return ret;
}
