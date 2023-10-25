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

static int current = 0;
static KeyStruct buffer[KB_BUF_SIZE];

void printBool(boolean b) {
  if (b) ncPrint("True");
  else ncPrint("False");
}

void printKeyStruct(KeyStruct k) {
  char c[2] = {k.code, 0};
  ncPrint(c);
  ncPrint(": { ");
  ncPrint("shift: ");
  printBool(k.md.shiftPressed);
  ncPrint(", ");
  ncPrint("ctrl: ");
  printBool(k.md.ctrlPressed);
  ncPrint(", ");
  ncPrint("caps lock: ");
  printBool(k.md.capsLockPressed);
  ncPrint(", ");
  ncPrint("alt: ");
  printBool(k.md.altPressed);
  ncPrint(" }");
  ncNewline();
}

void printBuffer() {
  for (int i = 0; i < KB_BUF_SIZE; ++i) {
    printKeyStruct(buffer[i]);
  }
}

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
    copyModifierKeys(md, &key.md);
    printKeyStruct(key);
    buffer[current++] = key;
    current %= KB_BUF_SIZE;
  }
}

void copyModifierKeys(ModifierKeys src, ModifierKeys* dest) {
  dest->shiftPressed = src.shiftPressed;
  dest->ctrlPressed = src.ctrlPressed;
  dest->altPressed = src.altPressed;
  dest->capsLockPressed = src.capsLockPressed;
}

void readKbBuffer(KeyStruct buf[], int len) {
  for (int i = 0; i < len; ++i) {
    buf[i] = buffer[(current + i)%KB_BUF_SIZE];
  }
}
