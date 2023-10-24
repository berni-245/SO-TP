#include <interruptions.h>
#include <stdint.h>

InterruptionDescriptor *idt = (InterruptionDescriptor *)0;

void setupIdtEntry(int index, InterruptionFunction irqHandler) {
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
  picMask(TIMER_TICK_MASK /* & KEYBOARD_MASK */);
  enableInterruptions();
}

static InterruptionFunction interruptions[] = {
  timerTick,
};
void irqDispatcher(uint8_t index) {
  interruptions[index]();
}
