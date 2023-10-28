#include <booleans.h>
#include <keyboard.h>
#include <layouts.h>
#include <naiveConsole.h>
#include <stdint.h>

void copyModifierKeys(ModifierKeys src, ModifierKeys* dest);

static const char* codeMap = layoutMaps[0];
static const char* shiftCodeMap = layoutShiftMaps[0];

void setLayout(KbLayout layout) {
  codeMap = layoutMaps[layout];
}

static int writeIdx = 0;
static int readIdx = 0;
boolean canRead = False;
static KeyStruct buffer[KB_BUF_SIZE];

static ModifierKeys md = {False};
void readKeyToBuffer() {
  uint8_t code = readKeyCode();
  KeyStruct key;
  switch (code) {
  case LEFT_SHIFT:
    md.leftShiftPressed = True;
    break;
  case LEFT_SHIFT + 0x80:
    md.leftShiftPressed = False;
    break;
  case RIGHT_SHIFT:
    md.rightShiftPressed = True;
    break;
  case RIGHT_SHIFT + 0x80:
    md.rightShiftPressed = False;
    break;
  case CTRL:
    md.ctrlPressed = True;
    break;
  case CTRL + 0x80:
    md.ctrlPressed = False;
    break;
  case ALT:
    md.altPressed = True;
    break;
  case ALT + 0x80:
    md.altPressed = False;
    break;
  case CAPS_LOCK:
    md.capsLockPressed = !md.capsLockPressed;
    break;
  default:
    if (code < 0 || code >= LAYOUT_SIZE) return;
    key.code = code;
    if (md.leftShiftPressed || md.rightShiftPressed) key.key = shiftCodeMap[code];
    else key.key = codeMap[code];
    if (key.key == 0) return;
    copyModifierKeys(md, &key.md);
    int prevWriteIdx = writeIdx;
    buffer[writeIdx++] = key;
    writeIdx %= KB_BUF_SIZE;
    if (readIdx == prevWriteIdx && canRead) readIdx = writeIdx;
    canRead = True;
  }
}

void copyModifierKeys(ModifierKeys src, ModifierKeys* dest) {
  dest->rightShiftPressed = src.rightShiftPressed;
  dest->ctrlPressed = src.ctrlPressed;
  dest->altPressed = src.altPressed;
  dest->capsLockPressed = src.capsLockPressed;
}

int readKbBuffer(KeyStruct buf[], int len) {
  int i = 0;
  while (canRead && i < len) {
    buf[i++] = buffer[readIdx];
    readIdx = (readIdx + 1) % KB_BUF_SIZE;
    if (readIdx == writeIdx) canRead = False;
  }
  return i;
}
