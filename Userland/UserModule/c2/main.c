/* sampleCodeModule.c */

#include <shell.h>
#include <syscalls.h>
#include <sysinfo.h>

#include <snake.h>

int main() {
  getSysInfo();

  // shell();

  // setFontColor(0xC0CAF5);
  // setBgColor(0x1A1B26);
  // setCursorColor(0xFFFF11);
  // clearScreen();
  // printChar('a');

  char* argv[] = {"shell", "test arg 1", "test arg 2"};
  sysCreateProcess(sizeof(argv) / sizeof(argv[0]), argv, shell);

  // char* argv2[] = {"shell2", "test 2 arg 1", "test 2 arg 2"};
  // sysCreateProcess(sizeof(argv2) / sizeof(argv2[0]), argv2, shell);

  // This should only run until the shell process begins, afterwards I don't
  // think this code will ever be reached again.
  while (1) sysHalt();
  // sysHalt();

  return 1;
}
