#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

#define KB_RAW_BUF_SIZE 5
#define KB_BUF_SIZE 10

typedef struct KeyCode {
  uint8_t row: 3;
  uint8_t col: 5;
  uint8_t shiftPressed: 1;
  uint8_t ctrlPressed: 1;
  uint8_t altPressed: 1;
  uint8_t capsLockPressed: 1;
  uint8_t altGrPressed: 1;
} KeyCode;

// typedef enum ModifierKeys {
//   
// } ModifierKeys;

typedef enum KbLayout {
  Key_0_0_pressed = 0x1,
  Key_0_0_released = 0x81,
  Key_0_1 = 0,
  Key_0_2 = 0,
  Key_0_3 = 0,
  Key_0_4 = 0,
  Key_0_5 = 0,
  Key_0_6 = 0,
  Key_0_7 = 0,
  Key_0_8 = 0,
  Key_0_9 = 0,
  Key_0_10 = 0,
  Key_0_11 = 0,
  Key_0_12 = 0,
  Key_0_13 = 0,
  Key_0_14 = 0,
} KbLayout;

extern uint8_t readKeyCode();
void readKeyToBuffer();
uint8_t* getKbBuffer();

#endif
