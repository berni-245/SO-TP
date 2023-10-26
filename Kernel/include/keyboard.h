#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "booleans.h"
#include <stdint.h>

#define KB_BUF_SIZE 10

typedef struct ModifierKeys {
  boolean shiftPressed: 1;
  boolean ctrlPressed: 1;
  boolean altPressed: 1;
  boolean capsLockPressed: 1;
  // boolean altGrPressed: 1;
} ModifierKeys;

typedef struct KeyStruct {
  char code;
  ModifierKeys md;
} KeyStruct;

typedef enum QwertyLayout {
  CAPS_LOCK = 0x3A,
  SHIFT = 0x2A,
  CTRL = 0x1D,
  ALT = 0x38,
} QwertyLayout;

typedef enum KbLayout {
  QWERTY_LATAM = 0,
  QWERTY_US,
} KbLayout;

extern uint8_t readKeyCode();
void readKeyToBuffer();
int readKbBuffer(KeyStruct buf[], int len);
void setLayout(KbLayout layout);

#endif
