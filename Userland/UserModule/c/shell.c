/* sampleCodeModule.c */

#include <errors.h>
#include <shell.h>
#include <shellCommands.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <sysinfo.h>
#include <syscalls.h>

static char screenBuffer[3000];
static int screenBufIdx = 0;
static int currentCommandIdx = 0;

// static char stdin[300];
// static int stdinIdx = 0;
static char stdout[300];

static int commandReturnCode = 0;

void newPrompt();
void addCommand(char* name, char* description, ShellFunction function);
ResultCode parseCommand();

int shell() {
  SystemInfo si;
  sysInfo(&si);

  // setBgColor(0x1A1B26);
  // setFontColor(0xC0CAF5);
  // clearScreen();

  KeyStruct buf[20];

  addCommand("help", "List all commands", help);
  addCommand("echo", "Print all arguments", echo);
  addCommand("$?", "Print previous command return code", getCommandReturnCode);
  addCommand("keyInfo", "Get pressed key info. Exit with ctrl+c", getKeyInfo);
  help();
  newPrompt();

  int read;
  while (1) {
    sysHalt();
    read = sysRead(buf, 20);
    for (int i = 0; i < read; ++i) {
      if (buf[i].md.ctrlPressed) {
        switch (buf[i].key) {
          case '+':
            sysSetFontSize(++si.fontSize);
            break;
          case '-':
            sysSetFontSize(--si.fontSize);
            break;
          case 'l':
            
            break;
        }
      } else {
        char key = buf[i].key;
        if (key == '\n') {
          screenBuffer[screenBufIdx++] = key;
          sysWriteCharNext(key);
          commandReturnCode = parseCommand();
          newPrompt();
        } else if (key == '\b') {
          if (screenBufIdx > currentCommandIdx) {
            sysWriteCharNext(key);
            screenBufIdx--;
          }
        } else {
          screenBuffer[screenBufIdx++] = key;
          // stdin[stdinIdx++] = key;
          sysWriteCharNext(key);
        }
      }
    }
  };

  return 1;
}

static char* prompt = " > ";
static char* errorPrompt = " >! ";
void newPrompt() {
  char* currentPrompt = (commandReturnCode == 0) ? prompt : errorPrompt;
  for (int i = 0; currentPrompt[i] != 0; ++i) {
    screenBuffer[screenBufIdx++] = currentPrompt[i];
  }
  currentCommandIdx = screenBufIdx;
  printString(currentPrompt);
}

static ShellCommand commands[50];
static int commandCount = 0;
void addCommand(char* name, char* description, ShellFunction function) {
  ShellCommand newCommand = {.name = name, .description = description, .function = function};
  commands[commandCount++] = newCommand;
}
ShellFunction getCommand(char* name) {
  for (int i = 0; i < commandCount; ++i) {
    if (strcmp(name, commands[i].name) == 0) return commands[i].function;
  }
  return NULL;
}

ResultCode parseCommand() {
  // Upto MAX_ARG_COUNT arguments (including the actual command)
  // of MAX_COMMAND_LEN characters each (+1 for null termination).
  char argv[MAX_ARG_COUNT][MAX_ARG_LEN + 1];
  int i = currentCommandIdx;
  int len = 0;
  int argc = 0;
  ShellFunction command;
  while (argc < MAX_ARG_COUNT) {
    if (screenBuffer[i] != ' ' && screenBuffer[i] != '\n' && len < MAX_ARG_LEN) {
      argv[argc][len++] = screenBuffer[i++];
    } else {
      argv[argc][len] = 0;
      if (len == MAX_ARG_LEN) {
        printf("Argument too long: %s...\n", argv[argc]);
        return ARGUMENT_TOO_LONG;
      }
      if (argc == 0) {
        if (argv[0][0] == 0) return 0;
        command = getCommand(argv[0]);
        if (command == NULL) {
          printf("Command not found: %s\n", argv[0]);
          return COMMAND_NOT_FOUND;
        }
      }
      ++argc;
      len = 0;
      if (screenBuffer[i] == '\n') {
        return command(argc, argv);
      }
      while (screenBuffer[++i] == ' ');
    }
  }
  printString("Too many arguments\n");
  return TOO_MANY_ARGUMENTS;
}

ResultCode echo(int argc, char argv[argc][MAX_ARG_LEN + 1]) {
  // Starts at 1 because first arg is the command name
  for (int i = 1; i < argc; ++i) {
    printf("%s ", argv[i]);
  }
  sysWriteCharNext('\n');
  return SUCCESS;
}

ResultCode getCommandReturnCode() {
  printf("%d\n", commandReturnCode);
  return SUCCESS;
}

ResultCode help() {
  printString("Available commands:\n");
  for (int i = 0; i < commandCount; ++i) {
    printf("\t- %s: %s\n", commands[i].name, commands[i].description);
  }
  return SUCCESS;
}

// ResultCode getKeyInfo() {
//   KeyStruct buf[20];
//   int read;
//   while (1) {
//     sysHalt();
//     read = sysRead(buf, 20);
//     if (buf[i].md.ctrlPressed) {
//       switch (buf[i].key) {
//         case '+':
//           sysSetFontSize(++si.fontSize);
//           break;
//         case '-':
//           sysSetFontSize(--si.fontSize);
//           break;
//         case 'l':
//           
//           break;
//       }
//     }
//   }
//   return SUCCESS;
// }
