#include <exceptions.h>
#include <registers.h>
#include <videoDriver.h>
#include <timer.h>

void zeroDivisionException(){
	clearScreen();
  printNextString("An exception occurred | ExceptionId: 0 - ZeroDivision Error\n\n");
  printExceptionRegisters();
  printNextString("Returning to shell in 10 seconds\n");
  sleep(10000);
}

void invalidOpcodeException(){
	clearScreen();
	printNextString("An exception occurred | ExceptionId: 6 - InvalidOpcode Error\n\n");
	printExceptionRegisters();
  printNextString("Returning to shell in 10 seconds\n");
  sleep(10000);
}
