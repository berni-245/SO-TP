#ifndef KEYBOARD_H
#define KEYBOARD_H


#include <stdint.h>

typedef struct ModifierKeys {
  uint8_t leftShiftPressed: 1;
  uint8_t rightShiftPressed: 1;
  uint8_t ctrlPressed: 1;
  uint8_t altPressed: 1;
  uint8_t capsLockPressed: 1;
} ModifierKeys;

typedef struct KeyStruct {
  uint8_t code;
  char key;
  ModifierKeys md;
} KeyStruct;


#endif
