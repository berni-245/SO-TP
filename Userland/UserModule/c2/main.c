/* sampleCodeModule.c */

#include <shell.h>
#include <syscalls.h>
#include <sysinfo.h>

int32_t main() {
  getSysInfo();

  const char* argv[] = {"shell", "test arg 1", "test arg 2"};
  while (true) {
    int32_t pid = sysCreateProcess(sizeof(argv) / sizeof(argv[0]), argv, shell);
    sysWaitPid(pid);
  }

  return 1;
}
