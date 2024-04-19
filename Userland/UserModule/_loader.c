/* _loader.c */
#include <stdint.h>

extern char userlandBss;
extern char endOfBinary;

int main();

void * memset(void * destiny, int32_t c, uint64_t length);

int _start() {
	//Clean BSS
	memset(&userlandBss, 0, &endOfBinary - &userlandBss);

	return main();
}


void * memset(void * destiation, int32_t c, uint64_t length) {
	uint8_t chr = (uint8_t)c;
	char * dst = (char*)destiation;

	while(length--)
		dst[length] = chr;

	return destiation;
}
