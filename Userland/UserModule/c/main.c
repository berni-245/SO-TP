/* sampleCodeModule.c */

#include <keyboard.h>
#include <stdint.h>
#include <sysinfo.h>

extern void sysHalt();
extern int sysMs();
extern void sysInfo(SystemInfo* si);
extern void sysSetLayout(int layoutIdx);
extern void sysSetFontSize(int fontsize);
extern int sysRead(KeyStruct* buf, int len);
extern int sysWrite(int x, int y, char c);
extern int sysWriteCharXY(int x, int y, char c);
extern int sysWriteCharNext(char c);
extern int sysWriteBuffer(int col, int row, const char buf[], int size);

int main() {
  SystemInfo si;
  sysInfo(&si);

  KeyStruct buf[20];

  int read, k = 0;
  while (1) {
    sysHalt();
    read = sysRead(buf, 20);
    for (int i = 0; i < read; ++i) {
      if (buf[i].key == '+' && buf[i].md.ctrlPressed) {
        sysSetFontSize(++si.fontSize);
      } else if (buf[i].key == '-' && buf[i].md.ctrlPressed) {
        sysSetFontSize(--si.fontSize);
      } else {
        sysWriteCharNext(buf[i].key);
      }
    }
  };

  return 1;
}
