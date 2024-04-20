#include <clock.h>
#include <interruptions.h>
#include <lib.h>
#include <moduleLoader.h>
#include <stdint.h>
#include <videoDriver.h>

// extern uint8_t kernelText;
// extern uint8_t kernelRodata;
// extern uint8_t kernelData;
extern uint8_t kernelBss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;

static const uint64_t PageSize = 0x1000;

typedef int (*EntryPoint)();

static EntryPoint const userModule = (EntryPoint)0x400000;
static EntryPoint const sampleDataModule = (EntryPoint)0x500000;

void clearBSS(void* bssAddress, uint64_t bssSize) {
  memset(bssAddress, 0, bssSize);
}

void* getStackBase() {
  // PageSize * 8 = The size of the stack itself, 32KiB
  // Subtract sizeof(uint64_t) to begin at the top of the stack.
  return (void*)((uint64_t)&endOfKernel + PageSize * 8 - sizeof(uint64_t));
}

void* initializeKernelBinary() {
  void* moduleAddresses[] = {userModule, sampleDataModule};

  loadModules(&endOfKernelBinary, moduleAddresses);

  clearBSS(&kernelBss, &endOfKernel - &kernelBss);

  setBinaryClockFormat();

  return getStackBase();
}

int main() {
  loadIdt();
  setFontGridValues();

  userModule();

  return 0;
}
