#include <keyboard.h>
#include <stdint.h>
#include <naiveConsole.h>

static uint8_t rawCodesBuffer[KB_RAW_BUF_SIZE];
static int currentRaw = 0;
static char buffer[KB_BUF_SIZE];

void readKeyToBuffer() {
  uint8_t code = readKeyCode();
  ncPrintHex(code);
  rawCodesBuffer[currentRaw++] = code;
  if (currentRaw >= KB_BUF_SIZE) currentRaw = 0;
}

uint8_t* getKbBuffer() {
  return rawCodesBuffer;
}
