#ifndef SHELL_COMMANDS_H
#define SHELL_COMMANDS_H

#include <shell.h>

typedef CommandResult (*ShellFunction)(int argc, char[argc][MAX_ARG_LEN]);

typedef struct ShellCommand {
  char* name;
  char* description;
  ShellFunction function;
} ShellCommand;

CommandResult commandEcho(int argc, char argv[argc][MAX_ARG_LEN]);
CommandResult commandGetReturnCode();
CommandResult commandHelp();
CommandResult commandGetKeyInfo();
CommandResult commandRand(int argc, char argv[argc][MAX_ARG_LEN]);
CommandResult commandLayout(int argc, char argv[argc][MAX_ARG_LEN]);
CommandResult commandSetColors(int argc, char (*argv)[MAX_ARG_LEN]);
CommandResult commandSysInfo();
CommandResult commandSnake(int argc, char argv[argc][MAX_ARG_LEN]);
CommandResult commandTest();

#endif
