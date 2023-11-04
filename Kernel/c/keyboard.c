#include <booleans.h>
#include <keyboard.h>
#include <layouts.h>
#include <stdint.h>

void copyModifierKeys(ModifierKeys src, ModifierKeys* dest);

static KbLayout kbLayout = 0;

void setLayout(KbLayout layout) {
  kbLayout = layout;
}

KbLayout getLayout() {
  return kbLayout;
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
    md.capsLockActive = !md.capsLockActive;
    break;
  default:
    if (code < 0 || code >= LAYOUT_SIZE) return;
    key.code = code;
    // This makes capslock virtually equivalent to shift, meaning all symbols will get 
    // converted, not only letters. That's not the standard behaviour but I actually like it.
    if (
      md.capsLockActive && !(md.leftShiftPressed || md.rightShiftPressed) ||
      !md.capsLockActive && (md.leftShiftPressed || md.rightShiftPressed)
    ) key.key = layoutShiftMaps[kbLayout][code];
    else key.key = layoutMaps[kbLayout][code];
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
  dest->capsLockActive = src.capsLockActive;
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
