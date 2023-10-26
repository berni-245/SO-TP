#include "keyboard.h"
#include <timer.h>
#include <interruptions.h>
#include <stdint.h>
#include <string.h>
#include <lib.h>
#include <moduleLoader.h>
#include <naiveConsole.h>

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

	return getStackBase();
}

int main()
{	
  loadIdt();
  ncClear();
	ncPrint("Start");

  KeyStruct buf[4];

  int read;
  while (1) {
    haltTillNextInterruption();
    if (getTicks() % 100 == 0) {
      read = readKbBuffer(buf, 4);
      ncNewline();
      ncPrint("Buffer 1:");
      ncNewline();
      printBuffer(buf, read);
    }
  };

  // setLayout(QWERTY_US);
	sampleCodeModule();

	return 0;
}





//        write 1234567          
//  read                read      
//   v        read 3     v       read = 1234
//   1234567  ----->  1234567        
//          ^                ^     
//        write            write   
//                               
//                               
//           write 890abcde            
//     read                read      
//      v         read 8     v           read = de67890a Mal!!! Debería ser 67890abvc
//   abcde67890   ------>   abcde67890        
//        ^                      ^ 
//      write                   write
//                               
//                               
//           write 890abcde            
//     read                read      
//      v         read 8     v           read = de67890a Mal!!! Debería ser 67890abvc
//   abcde67890   ------>   abcde67890        
//        ^                      ^ 
//      write                   write
//                               
//                               
//                       v       
//   1234567  ----->  abcd567890     
//                               
//                               
//                               
//                               
//                               













