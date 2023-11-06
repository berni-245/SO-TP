/* sampleCodeModule.c */

#include <shell.h>
#include <stdlib.h>
#include <syscalls.h>
#include <sysinfo.h>

int main() {
  getSysInfo();

  shell();

  return 1;
}
