#include <booleans.h>
#include <keyboard.h>
#include <naiveConsole.h>
#include <stdint.h>

void copyModifierKeys(ModifierKeys src, ModifierKeys* dest);

#define LAYOUT_SIZE 0x57

static char layoutMaps[][LAYOUT_SIZE] = {
  { // QWERTY_LATAM
    0, 27,
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '\'', 0, '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
    0, '+', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
    0, '{', 0, 0, '}',
    'z', 'x', 'c', 'v', 'b', 'n', 'm',
    ',', '.', '-', 0, 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, '<',
  },
  { // QWERTY_US
    0, 27,
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
    '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
    ';', '\'', 0, 0, 0,
    'z', 'x', 'c', 'v', 'b', 'n', 'm',
    ',', '.', '/', 0, 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0,
  }
};

static char* codeMap = layoutMaps[0];

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
  case SHIFT:
    md.shiftPressed = True;
    break;
  case SHIFT + 0x80:
    md.shiftPressed = False;
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
    // ncPrintHex(code);
    if (code < 0 || code >= LAYOUT_SIZE) return;
    key.code = codeMap[code];
    if (key.code == 0) return;
    copyModifierKeys(md, &key.md);
    // printKeyStruct(key);
    int prevWriteIdx = writeIdx;
    buffer[writeIdx++] = key;
    writeIdx %= KB_BUF_SIZE;
    if (readIdx == prevWriteIdx && canRead) readIdx = writeIdx;
    canRead = True;
  }
}

void copyModifierKeys(ModifierKeys src, ModifierKeys* dest) {
  dest->shiftPressed = src.shiftPressed;
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
