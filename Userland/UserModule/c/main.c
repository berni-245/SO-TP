/* sampleCodeModule.c */

#include <shell.h>
#include <stdlib.h>
#include <syscalls.h>
#include <sysinfo.h>

int main() {
  sysInfo(&systemInfo);

  shell();

  return 1;
}
