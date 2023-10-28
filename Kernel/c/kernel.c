#include <asciiBitFields.h>
#include <keyboard.h>
#include <timer.h>
#include <interruptions.h>
#include <stdint.h>
#include <string.h>
#include <lib.h>
#include <moduleLoader.h>
#include <videoDriver.h>

extern uint8_t * ascii_bit_fields;
extern int ascii_bf_width;
extern int ascii_bf_height;
extern int ascii_bf_count;

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

	clearBSS(&bss, &endOfKernel - &bss);

	return getStackBase();
}

// This should go in userland //////
#define SCREEN_BUF_SIZE 1000
static char screenBuf[SCREEN_BUF_SIZE];

static int nextGridPos = 0;
// static int cursorX = 0;
// static int cursorY = 0;
// void printNextChar(char c) {
//   printChar(cursorX, cursorY, c);
//   if (++cursorX >= this.maxPrintCol) {
//     this.printCol = 0;
//     if (++this.printRow >= this.maxPrintRow) {
//       console.warn("No more space in canvas. Resetting");
//       this.clearTestCanvas()
//     }
//   }
// }
///////////////////////////////////

int main()
{	
  loadIdt();

  KeyStruct buf[KB_BUF_SIZE];

  setBgColor(0x444444);
  clearScreen();

  setStrokeColor(0x0000FF);
  setFillColor(0xFF0000);
  setFontColor(0xFFFF00);

  strokeLine(40, 20, 1200, 700);
  setStrokeWidth(3);
  strokeRectangle(20, 20, 200, 150);
  setStrokeWidth(50);
  setStrokeColor(0x00FF00);
  // strokeRectangle(200, 200, 200, 200);
  fillRectangle(200, 200, 200, 200);

  setStrokeWidth(3);
  setStrokeColor(0x000000);
  strokeLine(180, 200, 180, 400);
  strokeLine(200, 180, 400, 180);
  
  increaseFont();
  printBuffer(0, 0, "Hola Carola", 11);

  int read, k = 0;
  while (1) {
    haltTillNextInterruption();
    read = readKbBuffer(buf, KB_BUF_SIZE);
    for (int i = 0; i < read; ++i, k = (k + 1) % SCREEN_BUF_SIZE) {
      if (buf[i].key == '+' && buf[i].md.ctrlPressed) {
        increaseFont();
      } else if (buf[i].key == '-' && buf[i].md.ctrlPressed) {
        decreaseFont();
      } else {
        clearScreen();
        screenBuf[k] = buf[i].key;
        printChar(10, 1, screenBuf[k]);
      }
    }
  };

  // setLayout(QWERTY_US);
	// sampleCodeModule();

	return 0;
}
