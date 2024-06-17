#include <keyboard.h>
#include <layouts.h>
#include <pipes.h>
#include <stdbool.h>
#include <stdint.h>

static KbLayout kbLayout = 0;

void setLayout(KbLayout layout) {
  kbLayout = layout;
}

KbLayout getLayout() {
  return kbLayout;
}

#define EOF -1

// There is a problem where some key combinations are not detected, for example
// left-shift + right-shift + p or + b don't get printed. But the combination
// does get detected while debuggin so I think it's an issue with how interruptions
// are handled internally.
static ModifierKeys md = {false};
void readKeyToBuffer() {
  uint8_t code = readKeyCode();
  char c;
  switch (code) {
  case LEFT_SHIFT:
    md.leftShiftPressed = true;
    break;
  case LEFT_SHIFT + 0x80:
    md.leftShiftPressed = false;
    break;
  case RIGHT_SHIFT:
    md.rightShiftPressed = true;
    break;
  case RIGHT_SHIFT + 0x80:
    md.rightShiftPressed = false;
    break;
  case CTRL:
    md.ctrlPressed = true;
    break;
  case CTRL + 0x80:
    md.ctrlPressed = false;
    break;
  case ALT:
    md.altPressed = true;
    break;
  case ALT + 0x80:
    md.altPressed = false;
    break;
  case CAPS_LOCK:
    md.capsLockActive = !md.capsLockActive;
    break;
  default:
    if (code >= LAYOUT_SIZE) return;
    // This makes capslock virtually equivalent to shift, meaning all symbols will get
    // converted, not only letters. That's not the standard behaviour but I actually like it.
    if (md.capsLockActive != (md.leftShiftPressed || md.rightShiftPressed)) 
      c = layoutShiftMaps[kbLayout][code];
    else c = layoutMaps[kbLayout][code];
    if (c == 0) return;
    if (md.ctrlPressed == true) {
      if (c == 'C' || c == 'c') {
        killCurrentProcessInForeground();
        return;
      } else if (c == 'D' || c == 'd') {
        c = EOF;
      }
    }
    writeStdin(c);
  }
}

void getModKeys(ModifierKeys* dest) {
  dest->leftShiftPressed = md.leftShiftPressed;
  dest->rightShiftPressed = md.rightShiftPressed;
  dest->ctrlPressed = md.ctrlPressed;
  dest->altPressed = md.altPressed;
  dest->capsLockActive = md.capsLockActive;
}
