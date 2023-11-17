#ifndef SHELL_COMMANDS_H
#define SHELL_COMMANDS_H

#include <circularBuffer.h>
#include <colors.h>
#include <draw.h>
#include <utils.h>
#include <snake.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sysinfo.h>
#include <syscalls.h>

#define MAX_ARG_COUNT 20
#define MAX_ARG_LEN 50 + 1
#define MAX_COMMAND_COUNT 50
#define MAX_HISTORY_LEN 50

typedef enum {
  SUCCESS = 0,
  TOO_MANY_ARGUMENTS,
  ARGUMENT_TOO_LONG,
  COMMAND_NOT_FOUND,
  MISSING_ARGUMENTS,
  ILLEGAL_ARGUMENT,
  OUT_OF_BOUNDS,
} ExitCode;
static const char* const CommandResultStrings[] = {
  [SUCCESS] = "Success",
  [TOO_MANY_ARGUMENTS] = "Too many arguments",
  [ARGUMENT_TOO_LONG]  = "Argument too long",
  [COMMAND_NOT_FOUND]  = "Command not found",
  [MISSING_ARGUMENTS]  = "Missing arguments",
  [ILLEGAL_ARGUMENT]  = "Illegal argument",
  [OUT_OF_BOUNDS]  = "Argument of bounds",
};


typedef ExitCode (*ShellFunction)(int argc, char[argc][MAX_ARG_LEN]);

/*
typedef struct {
  // Set by user
  char* name;
  bool required;

  // Set by parseCommandArgs
  bool found;
} CommandArgument;
typedef struct {
  // Set by user
  char* shortName;
  char* longName;
  char* description;
  bool hasArgument;
  bool required;

  // Set by parseCommandOpts
  char argument[MAX_ARG_LEN];
  bool found;
} CommandOption;
*/

typedef struct ShellCommand {
  char* name;
  char* description;
  ShellFunction function;
  // CommandArgument arguments[MAX_ARG_COUNT];
  // CommandOption options[];
} ShellCommand;

void newPrompt();
void incFont();
void decFont();
void addCommand(char* name, char* description, ShellFunction function);
void setShellColors(uint32_t fontColor, uint32_t bgColor, uint32_t cursorColor);
void autocomplete();
void deleteWord();
void historyPush();
void historyPrev();
void historyNext();
// void parseCommandOpts(int argc, char argv[argc][MAX_ARG_LEN], int flagCount, CommandOption flags[]);

ExitCode parseCommand();
ExitCode commandEcho(int argc, char argv[argc][MAX_ARG_LEN]);
ExitCode commandGetReturnCode();
ExitCode commandRealTime();
ExitCode commandHelp();
ExitCode commandGetKeyInfo();
ExitCode commandRand(int argc, char argv[argc][MAX_ARG_LEN]);
ExitCode commandLayout(int argc, char argv[argc][MAX_ARG_LEN]);
ExitCode commandSetColors(int argc, char (*argv)[MAX_ARG_LEN]);
ExitCode commandSysInfo();
ExitCode commandGetRegisters(int argc, char argv[argc][MAX_ARG_LEN]);
ExitCode commandSnake(int argc, char argv[argc][MAX_ARG_LEN]);
ExitCode commandTest();
ExitCode commandZeroDivisionError();
ExitCode commandInvalidOpcodeError();

#endif
