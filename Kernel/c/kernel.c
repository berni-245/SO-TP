#include <asciiBitFields.h>
#include <keyboard.h>
#include <timer.h>
#include <interruptions.h>
#include <stdint.h>
#include <string.h>
#include <lib.h>
#include <moduleLoader.h>
#include <clock.h>
#include <pcSpeaker.h>
#include <videoDriver.h>
#include <naiveConsole.h>
#include <registers.h>
#include <exceptions.h>

extern uint8_t text;
extern uint8_t rodata;
extern uint8_t data;
extern uint8_t bss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;

static const uint64_t PageSize = 0x1000;

typedef int (*EntryPoint)();

static EntryPoint const userModule = (EntryPoint)0x400000;
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
		userModule,
		sampleDataModule
	};

	loadModules(&endOfKernelBinary, moduleAddresses);

	clearBSS(&bss, &endOfKernel - &bss);

	setBinaryClockFormat();

	return getStackBase();
}

int main()
{	
  loadIdt();
  setFontGridValues();

  setBgColor(0x1A1B26);
  setFontColor(0xC0CAF5);
  clearScreen();
  setLayout(QWERTY_US);
  sleep(2000);
  Register * registers = getRegisters();
  for(int i = 0; i < REGISTER_QUANTITY; i++){
    printNextString(registers[i].name); printNextChar(' '); printNextHexWithPadding(registers[i].value); printNextChar(' ');
    if(i%4 == 0)
      printNextChar('\n');
  }
  sleep(1000);
  zeroDivisionException();
  sleep(1000);
  invalidOpcodeException();

  // int test = 3/0; // zeroDivision Error
  //__asm__ ("ud2"); // invalidOpcode Error
  userModule();
  
  // printCharXY(500, 500, 'X', 8);

  // KeyStruct buf[20];
  // int read;
  // while (1) {
  //   haltTillNextInterruption();
  //   read = readKbBuffer(buf, 20);
  //   for (int i = 0; i < read; ++i) {
  //     if (buf[i].key == '+' && buf[i].md.ctrlPressed) {
  //       increaseFont(); 
  //     } else if (buf[i].key == '-' && buf[i].md.ctrlPressed) {
  //       decreaseFont();
  //     } else {
  //       printNextChar(buf[i].key);
  //     }
  //   }
  // };

	return 0;
}
