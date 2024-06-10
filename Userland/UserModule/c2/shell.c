#include "syscalls.h"
#include <array.h>
#include <circularHistoryBuffer.h>
#include <shellUtils.h>

extern uint8_t bss;

CircularHistoryBuffer commandHistory;
int currentCommandIdx = 0;

int commandReturnCode = 0;
static int currentPromptLen = 0;

int commandCount = 0;
Array currentCommand;

void freeArrayPtr(Array* ele) {
  Array_free(*ele);
}

int shell() {
  setShellColors(0xC0CAF5, 0x1A1B26, 0xFFFF11);
  clearScreen();

  commandCount = 0;
  currentCommand = Array_initialize(sizeof(char), 100, NULL, NULL);
  commandHistory = CHB_initialize(sizeof(Array), MAX_HISTORY_LEN, (FreeEleFn)freeArrayPtr, NULL);

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
  addCommand("testProcesses", "Checks the process creation, blocking, unblocking and destruction", commandTestProcesses);
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
  int i = Array_getLen(currentCommand);
  while (i--) printChar('\b');
  Array_clear(currentCommand);
}

void historyCopy(Array argv) {
  clearLine();
  int argc = Array_getLen(argv);
  for (int i = 0; i < argc; ++i) {
    const char* arg = Array_getVanillaArray(*(Array*)Array_get(argv, i));
    for (int i = 0; arg[i] != 0; ++i) {
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
    // clearLine();
    return;
  }
  historyCopy(*argv);
}

int getCurrentChar() {
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
  int len = Array_getLen(currentCommand);
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

ShellCommand commands[MAX_COMMAND_COUNT];
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
  const char* cc = Array_getVanillaArray(currentCommand);
  int ccLen = Array_getLen(currentCommand);
  for (int i = 0; i < commandCount; ++i) {
    char* command = commands[i].name;
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
  // printString(commands[matchIdx].name + len);
  for (int i = len; commands[matchIdx].name[i] != 0; ++i) {
    printChar(commands[matchIdx].name[i]);
    Array_push(currentCommand, commands[matchIdx].name + i);
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
  int argc = Array_getLen(argv);
  if (argc == 0) return;
  for (int i = 0; i < argc; ++i) {
    char end = 0;
    Array_push(*(Array*)Array_get(argv, i), &end);
  }
}

void setRealArgv(int argc, const char* realArgv[argc], Array argv) {
  for (int i = 0; i < argc; ++i) {
    realArgv[i] = Array_getVanillaArray(*(Array*)Array_get(argv, i));
  }
}

ExitCode parseCommand() {
  Array argv = Array_initialize(sizeof(Array), 10, (FreeEleFn)freeArrayPtr, NULL);
  Array argv2 = NULL;
  Array currentArgv = argv;
  ShellFunction command;
  ShellFunction command2 = NULL;
  Array arg = NULL;
  const char* cc = Array_getVanillaArray(currentCommand);
  int commandLength = Array_getLen(currentCommand);
  bool newWord = true;
  for (int i = 0; i < commandLength; ++i) {
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

  int argc = Array_getLen(argv);
  if (argc == 0) return SUCCESS;

  int ret = SUCCESS;
  if (argv2 != NULL) {
    --argc; // Dont take the pipe into account.
    int argc2 = Array_getLen(argv2);
    command2 = verifyCommand(argv2);
    if (command2 == NULL) ret = COMMAND_NOT_FOUND;
    else {
      command = verifyCommand(argv);
      if (command != NULL) {
        setArgsNullTerminaor(argv);
        setArgsNullTerminaor(argv2);
        const char* realArgv[argc];
        const char* realArgv2[argc2];
        setRealArgv(argc, realArgv, argv);
        setRealArgv(argc2, realArgv2, argv2);
        int pipe = sysPipeInit();
        ProcessPipes pipes = {.write = pipe, .read = stdin, .err = stderr};
        int pid = sysCreateProcessWithPipeSwap(argc, realArgv, command, pipes);
        pipes.write = stdout;
        pipes.read = pipe;
        int pid2 = sysCreateProcessWithPipeSwap(argc2, realArgv2, command2, pipes);
        ret = sysWaitPid(pid);
        sysDestroyPipe(pipe);
        ret = sysWaitPid(pid2);
      } else ret = COMMAND_NOT_FOUND;
    }
    Array_concat(argv, argv2);
    Array_free(argv2);
  } else {
    command = verifyCommand(argv);
    if (command != NULL) {
      setArgsNullTerminaor(argv);
      const char* realArgv[argc];
      for (int i = 0; i < argc; ++i) {
        realArgv[i] = Array_getVanillaArray(*(Array*)Array_get(argv, i));
      }
      if (strcmp(realArgv[argc - 1], "&") == 0) {
        int pid = sysCreateProcess(Array_getLen(argv) - 1, realArgv, command);
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
