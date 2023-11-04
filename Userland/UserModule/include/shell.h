#ifndef SHELL_H
#define SHELL_H

#define MAX_ARG_COUNT 20
#define MAX_ARG_LEN 50 + 1

extern char screenBuffer[3000];
extern int screenBufIdx;
typedef enum {
  SUCCESS = 0,
  TOO_MANY_ARGUMENTS,
  ARGUMENT_TOO_LONG,
  COMMAND_NOT_FOUND,
  MISSING_ARGUMENTS,
  ILLEGAL_ARGUMENT,
  OUT_OF_BOUNDS,
} CommandResult;
int shell();

#endif
