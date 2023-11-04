#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "booleans.h"
#include <stdint.h>

#define KB_BUF_SIZE 20

typedef struct ModifierKeys {
  boolean leftShiftPressed: 1;
  boolean rightShiftPressed: 1;
  boolean ctrlPressed: 1;
  boolean altPressed: 1;
  boolean capsLockActive: 1;
  // boolean altGrPressed: 1;
} ModifierKeys;

typedef struct KeyStruct {
  uint8_t code;
  char key;
  ModifierKeys md;
} KeyStruct;

enum ModKeyCodes {
  CAPS_LOCK = 0x3A,
  LEFT_SHIFT = 0x2A,
  RIGHT_SHIFT = 0x36,
  CTRL = 0x1D,
  ALT = 0x38,
};

typedef enum KbLayout {
  QWERTY_LATAM = 0,
  QWERTY_US,
} KbLayout;

extern uint8_t readKeyCode();
void readKeyToBuffer();
int readKbBuffer(KeyStruct buf[], int len);
void setLayout(KbLayout layout);
KbLayout getLayout();

#endif
