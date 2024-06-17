#ifndef SHELL_COMMANDS_H
#define SHELL_COMMANDS_H

#include <array.h>
#include <circularBuffer.h>
#include <circularHistoryBuffer.h>
#include <colors.h>
#include <draw.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sysinfo.h>
#include <utils.h>

#define MAX_HISTORY_LEN 20

typedef enum {
  SUCCESS = 0,
  TOO_MANY_ARGUMENTS,
  ARGUMENT_TOO_LONG,
  COMMAND_NOT_FOUND,
  MISSING_ARGUMENTS,
  ILLEGAL_ARGUMENT,
  OUT_OF_BOUNDS,
  PROCESS_FAILURE,
  NO_MEMORY_AVAILABLE,
} ExitCode;
static const char* const CommandResultStrings[] = {
    "Success",          "Too many arguments", "Argument too long", "Command not found",  "Missing arguments",
    "Illegal argument", "Argument of bounds", "Process failure",   "No memory available"
};

typedef void (*ShellFunction)(int32_t argc, char* [argc]);

typedef struct ShellCommand {
  char* name;
  char* description;
  ShellFunction function;
} ShellCommand;

extern int32_t commandReturnCode;
extern Array commands;

void newPrompt();
void incFont();
void decFont();
void clearLine();
void clearScreenKeepCommand();
void addCommand(char* name, char* description, ShellFunction function);
void setShellColors(uint32_t fontColor, uint32_t bgColor, uint32_t cursorColor);
void autocomplete();
void deleteWord();
void historyPrev();
void historyNext();
void resetHistoryCurrentVals();

ExitCode parseCommand();
void commandEcho(int32_t argc, char* argv[argc]);
void commandGetReturnCode();
void commandRealTime();
void commandHelp(int32_t argc, char* argv[argc]);
void commandGetKeyInfo();
void commandRand(int32_t argc, char* argv[argc]);
void commandLayout(int32_t argc, char* argv[argc]);
void commandSetColors(int32_t argc, char* argv[argc]);
void commandSysInfo();
void commandGetRegisters(int32_t argc, char* argv[argc]);
void commandTest();
void commandZeroDivisionError();
void commandInvalidOpcodeError();
void commandPs();
void commandCreateSemaphore(int32_t argc, char* argv[argc]);
void commandDestroySemaphore(int32_t argc, char* argv[argc]);
void commandTestSem(int32_t argc, char* argv[argc]);
void commandChangeProcess(); // Arreglar. Esto no es un command...
void commandKill(int32_t argc, char* argv[argc]);
void commandGetMemoryState(int32_t argc, char* argv[argc]);
void commandLoop(int32_t argc, char* argv[argc]);
void commandPhylo(int32_t argc, char* argv[argc]);
void commandNice(int32_t argc, char* argv[argc]);
void commandTestPipes(int32_t argc, char* argv[argc]);
void commandBlock(int32_t argc, char* argv[argc]);
void commandUnBlock(int32_t argc, char* argv[argc]);
void commandTestMM(int32_t argc, char* argv[]);
void commandTestPriority(int32_t argc, char* argv[argc]);
void commandTestProcesses(int32_t argc, char* argv[]);
void commandCat();
void commandWordCount();
void commandFilterVocals();
void commandSuperSecret();

#endif
