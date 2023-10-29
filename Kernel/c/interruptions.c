#include <interruptions.h>
#include <keyboard.h>
#include <stdint.h>
#include <syscalls.h>

InterruptionDescriptor *idt = (InterruptionDescriptor *)0;

void setupIdtEntry(int index, void* irqHandler) {
  uint64_t offset = (uint64_t)irqHandler;
  idt[index].offset_0_15 = offset & 0xFFFF;
  idt[index].offset_16_31 = (offset >> 16) & 0xFFFF;
  idt[index].offset_32_63 = (offset >> 32) & 0xFFFFFFFF;
  idt[index].segmentSelector = 0x08; // Defined by Pure64
  idt[index].accessPrivileges = ACS_INT;
  idt[index].empty1 = 0;
  idt[index].empty2 = 0;
}

void loadIdt() {
  setupIdtEntry(0x20, irq00Handler);
  setupIdtEntry(0x21, irq01Handler);
  setupIdtEntry(0x80, syscallHandler);
  picMask(TIMER_TICK_MASK & KEYBOARD_MASK);
  enableInterruptions();
}

static InterruptionFunction interruptions[255] = {
  timerTick,
  readKeyToBuffer,
};

void irqDispatcher(uint8_t index) {
  interruptions[index]();
}
