#include "keyboard.h"
#include <timer.h>
#include <interruptions.h>
#include <stdint.h>
#include <string.h>
#include <lib.h>
#include <moduleLoader.h>
#include <naiveConsole.h>
#include <clock.h>
#include <pcSpeaker.h>
#include <registers.h>

extern uint8_t text;
extern uint8_t rodata;
extern uint8_t data;
extern uint8_t bss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;

static const uint64_t PageSize = 0x1000;

typedef int (*EntryPoint)();

static EntryPoint const sampleCodeModule = (EntryPoint)0x400000;
static EntryPoint const sampleDataModule = (EntryPoint)0x500000;

void clearBSS(void * bssAddress, uint64_t bssSize)
{
	memset(bssAddress, 0, bssSize);
}

void * getStackBase()
{
	return (void*)(
		(uint64_t)&endOfKernel
		+ PageSize * 8				//The size of the stack itself, 32KiB
		- sizeof(uint64_t)			//Begin at the top of the stack
	);
}

void * initializeKernelBinary()
{
	void * moduleAddresses[] = {
		sampleCodeModule,
		sampleDataModule
	};

	loadModules(&endOfKernelBinary, moduleAddresses);
	ncNewline();
	ncNewline();

	clearBSS(&bss, &endOfKernel - &bss);

	setBinaryClockFormat();

	return getStackBase();
}

void test() {
	sleep(1000);
	Register * registers = getRegisters();
	for(int i = 1; i <= REGISTER_QUANTITY; i++){
		ncPrint(" ");
		ncPrint(registers[i-1].name);
		ncPrint(" ");
		ncPrintHex(registers[i-1].value);
		ncPrint(" ");
		if(i%4 == 0)
			ncNewline();
	}
	test();
}

#define bufSize 10
int main()
{	
  loadIdt();
  ncClear();
	ncPrint("Start");

  KeyStruct buf[bufSize];

  setLayout(QWERTY_US);

  test();

  int read;
  while (getCurrentMinutes() < 50) {
	int t = getMs();
    haltTillNextInterruption();
    read = readKbBuffer(buf, bufSize);
    for (int i = 0; i < read; ++i) {
		ncNewline();
		ncNewline();
	  if(buf->code == 0x26){
		Register * registers = getRegisters();
		for(int i = 1; i <= REGISTER_QUANTITY; i++){
			ncPrint(" ");
			ncPrint(registers[i-1].name);
			ncPrint(" ");
			ncPrintHex(registers[i-1].value);
			ncPrint(" ");
			if(i%4 == 0)
				ncNewline();
		}
	  }
    //   ncPrintChar(buf[i].key);
    }
  };

  sleep(1000);

  while (1) {
	int t = getMs();
    haltTillNextInterruption();
    read = readKbBuffer(buf, bufSize);
    for (int i = 0; i < read; ++i) {
		ncNewline();
		ncNewline();
	  if(buf->code == 0x26){
		Register * registers = getRegisters();
		for(int i = 1; i <= REGISTER_QUANTITY; i++){
			ncPrint(" ");
			ncPrint(registers[i-1].name);
			ncPrint(" ");
			ncPrintHex(registers[i-1].value);
			ncPrint(" ");
			if(i%4 == 0)
				ncNewline();
		}
	  }
    //   ncPrintChar(buf[i].key);
    }
  };

	// sampleCodeModule();

	return 0;
}
