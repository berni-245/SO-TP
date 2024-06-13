#include <array.h>
#include <shellUtils.h>
#include <stdlib.h>
#include <syscalls.h>

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

#define COMMANDS_PER_PAGE 10
void commandHelp(int argc, char* argv[argc]) {
  int commandCount = arrayGetLen(commands);
  // Rounded up number of pages.
  int nbrPages = (commandCount + COMMANDS_PER_PAGE - 1) / COMMANDS_PER_PAGE;
  int page = 1;
  if (argc > 1) page = strToInt(argv[1]);

  if (page > nbrPages) {
    printString("No such page\n");
    sysExit(ILLEGAL_ARGUMENT);
  }

  printf("Available commands (Page %d/%d)\n", page, nbrPages);

  int startIdx = (page - 1) * COMMANDS_PER_PAGE;
  int end = startIdx + COMMANDS_PER_PAGE > commandCount ? commandCount : startIdx + COMMANDS_PER_PAGE;
  for (int i = startIdx; i < end; ++i) {
    ShellCommand* command = arrayGet(commands, i);
    printf("\t- %s: %s\n", command->name, command->description);
  }
  printf("Switch between pages with `help [pageNr]`\n");
  sysExit(SUCCESS);
}

void commandGetKeyInfo() {
  KeyStruct key;
  while (getKey(&key)) {
    printKey(&key);
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
    printf("%lu%s", randBetween(min, max), (count == 0) ? "" : ", ");
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
  clearScreenKeepCommand();
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
  clearScreenKeepCommand();
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
  printf("%3s, %-10s, %-9s, %-5s, %10s, %10s, %8s\n", "PID", "Name", "State", "Fg/Bg", "rsp", "rbp", "Priority");
  for (int i = 0; i < len; ++i) {
    PCB* pcb = pcbList + i;
    printf(
        "%3lu, %-10s, %-9s, %-5s, %p, %p, %8d\n", pcb->pid, pcb->name, pcb->state, pcb->location, pcb->rsp, pcb->rbp,
        pcb->priority
    );
  }
  sysFree(pcbList);
  sysExit(SUCCESS);
}

void commandGetPid() {
  printf("Current process pid: %lu\n", sysGetPid());
  sysExit(SUCCESS);
}

void commandKill(int argc, char* argv[argc]) {
  if (argc < 2) {
    printf("Usage: kill <pid>\n");
    sysExit(ILLEGAL_ARGUMENT);
  }
  int pid = strToInt(argv[1]);
  if (pid == 0) {
    printf("https://youtu.be/31g0YE61PLQ?si=G3tv2y_iw8InNCec\n");
    sysExit(ILLEGAL_ARGUMENT);
  }

  if (sysKill(pid)) sysExit(SUCCESS);
  else {
    printf("Process with pid %d not found or has already exited\n", pid);
    sysExit(OUT_OF_BOUNDS);
  }
}

void commandGetMemoryState(int argc, char* argv[argc]) {
  char* memState;
  if (argc > 1) {
    int pid = strToInt(argv[1]);
    memState = sysGetProcessMemoryState(pid);
  } else {
    memState = sysGetGlobalMemoryState();
  }
  if (memState == NULL) {
    printf("Either no process found for given pid or all the memory for current process is being used and memory for "
           "satate message cannot be allocated.\n");
    sysExit(ILLEGAL_ARGUMENT);
  }
  printf("%s\n", memState);
  sysFree(memState);
  sysExit(SUCCESS);
}

void commandLoop(int argc, char* argv[argc]) {
  if (argc < 2) {
    puts("Usage:");
    printf("\t\t%s <secs>\n", argv[0]);
    sysExit(MISSING_ARGUMENTS);
  }
  int secs = strToInt(argv[1]);
  printf("secs: %d\n", secs);
  if (secs > 0) {
    while (1) {
      sysSleep(secs * 1000);
      printf("Hola! Soy el proceso: %d\n", sysGetPid());
    }
  }
  sysExit(PROCESS_FAILURE);
}

void commandNice(int argc, char* argv[argc]) {
  if (argc < 3) {
    puts("Usage:");
    printf("\t\t%s <pid> <priority between 1-9>\n", argv[0]);
    sysExit(MISSING_ARGUMENTS);
  }
  int newPriority = strToInt(argv[2]);
  if (newPriority <= 0 || newPriority >= 10) {
    puts("Usage:");
    printf("\t\t%s <pid> <priority between 1-9>\n", argv[0]);
    sysExit(ILLEGAL_ARGUMENT);
  }

  sysChangePriority(strToInt(argv[1]), newPriority);
  sysExit(SUCCESS);
}

void pipeWriter(int argc, char* argv[argc]) {
  static char* words[] = {"0000000", "1111111", "2222222", "3333333", "4444444",
                          "5555555", "6666666", "7777777", "8888888", "9999999"};
  static int wordsLen = sizeof(words) / sizeof(*words);
  static int bufLen = 400;
  char buf[bufLen];
  for (int i = 0, j = 0; i < bufLen; ++j) {
    i += strcpy(buf + i, words[j % wordsLen]);
  }
  // int writeLen = bufLen / 20;
  int len = 0;
  // ProcessPipes pipes = sysGetPipes();
  while (len < bufLen) {
    // int l = sysWrite(pipes.write, buf + len, writeLen);
    int l = printf("%s", buf);
    if (l < 0) {
      printf("%s - %lu: pipe not found, exiting...", argv[0], sysGetPid());
      sysExit(PROCESS_FAILURE);
    }
    len += l;
    sleep(1000);
  }
  sysExit(SUCCESS);
}
void pipeReader(int argc, char* argv[argc]) {
  char buf[200];
  int i = 0, tot = 0;
  ProcessPipes pipes = sysGetPipes();
  while (true) {
    i = sysRead(pipes.read, buf, 40);
    if (i < 0) {
      printf("%s - %lu: pipe not found, exiting...\n", argv[0], sysGetPid());
      sysExit(SUCCESS);
    }
    buf[i] = 0;
    tot += i;
    printf("Current read: %d - Total read: %d\n", i, tot);
    printf("%s\n", buf);
    sleep(1000);
  }
  sysExit(PROCESS_FAILURE);
}
void commandTestPipes(int argc, char* argv[argc]) {
  int pipe = sysPipeInit();
  printf("Using pipe: %d\n", pipe);
  const char* argv2[] = {"pipeWriter"};
  ProcessPipes pipes = {.write = pipe, .read = stdin, .err = stderr};
  int pidWriter = sysCreateProcessWithPipeSwap(1, argv2, pipeWriter, pipes);
  argv2[0] = "pipeReader";
  pipes.write = stdout;
  pipes.read = pipe;
  int pidReader = sysCreateProcessWithPipeSwap(1, argv2, pipeReader, pipes);

  sysWaitPid(pidWriter);
  printf("%s - %lu: Destroying pipe...\n", argv[0], sysGetPid());
  sleep(2000);
  if (!sysDestroyPipe(pipe)) {
    printf("Error destroying pipe: %d\n", pipe);
  }
  sysWaitPid(pidReader);

  sysExit(SUCCESS);
}

void commandDestroyPipe(int argc, char* argv[argc]) {
  if (argc < 2) {
    printf("Usage: %s <pip_id>\n", argv[0]);
    sysExit(ILLEGAL_ARGUMENT);
  }
  int pipe = strToInt(argv[1]);
  if (!sysDestroyPipe(pipe)) {
    printf("Error destroying pipe: %d\n", pipe);
    sysExit(ILLEGAL_ARGUMENT);
  }
  printf("Correctly destroyed pipe: %d\n", pipe);
  sysExit(SUCCESS);
}

void commandBlock(int argc, char* argv[argc]) {
  if (argc < 2) {
    printf("Usage: block <pid>\n");
    sysExit(ILLEGAL_ARGUMENT);
  }
  int pid = strToInt(argv[1]);

  sysBlock(pid);
  sysExit(SUCCESS);
}

void commandUnBlock(int argc, char* argv[argc]) {
  if (argc < 2) {
    printf("Usage: unblock <pid>\n");
    sysExit(ILLEGAL_ARGUMENT);
  }
  int pid = strToInt(argv[1]);

  if (pid == 0) {
    printf("https://youtu.be/31g0YE61PLQ?si=G3tv2y_iw8InNCec\n");
    sysExit(ILLEGAL_ARGUMENT);
  }

  sysUnBlock(pid);
  sysExit(SUCCESS);
}

void commandCat() {
  char c;
  while ((c = getChar()) != EOF) {
    if (printChar(c) < 0) sysExit(PROCESS_FAILURE);
  }
  sysExit(SUCCESS);
}

void commandWordCount() {
  char c;
  int words = 0;
  int lines = 0;
  bool inWord = false;
  bool lastReadNewLine = false;

  while ((c = getChar()) != EOF) {
    lastReadNewLine = false;
    if (c == ' ' || c == '\n') {
      if (inWord) {
        ++words;
        inWord = false;
      }
      if (c == '\n') {
        ++lines;
        lastReadNewLine = true;
      }
    } else inWord = true;
    if (printChar(c) < 0) sysExit(PROCESS_FAILURE);
  }
  if (inWord) ++words;
  if (!lastReadNewLine) ++lines;

  printf("\nWord count: %d\n", words);
  printf("Line count: %d\n", lines);

  sysExit(SUCCESS);
}

int charIsAVocal(char c) {
  return (
      c == 'a' || c == 'A' || c == 'e' || c == 'E' || c == 'i' || c == 'I' || c == 'o' || c == 'O' || c == 'u' ||
      c == 'U'
  );
}

void commandFilterVocals() {
  char c;
  while ((c = getChar()) != EOF) {
    if (!charIsAVocal(c)) {
      printf("%c", c);
    }
  }
  printf("\n");
  sysExit(SUCCESS);
}
