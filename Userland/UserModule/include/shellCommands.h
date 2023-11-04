#ifndef SHELL_COMMANDS_H
#define SHELL_COMMANDS_H

#include <errors.h>
#include <shell.h>

typedef ResultCode (*ShellFunction)(int argc, char[argc][MAX_ARG_LEN + 1]);

typedef struct ShellCommand {
  char* name;
  char* description;
  ShellFunction function;
} ShellCommand;

ResultCode echo(int argc, char argv[argc][MAX_ARG_LEN + 1]);
ResultCode getCommandReturnCode();
ResultCode help();
ResultCode getKeyInfo();

#endif
