//	Lab01.c	2014/06/16
#include <msp430.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "lab01.h"

unsigned long MCLK_HZ = 1050000;		// SMCLK frequency in Hz
unsigned long BPS = 9600;				// ASYNC serial baud rate

int main(void)
{
	lab01_init();

	/********** Part 1 **********/
	TERMINAL("\n\r** PART 1 *************");

	// largest signed int
	signed int signed_int = -1;
	signed_int = (unsigned)signed_int >> 1;
	TERMINAL1("Largest signed int: %d", signed_int);

	// most negative signed int
	TERMINAL1("Most negative signed int: %d", ~signed_int);

	// largest unsigned int
	unsigned int unsigned_int = 0;
	TERMINAL1("Largest unsigned int: %u", ~unsigned_int);

	//largest signed char
	signed char signed_char = 0;
	int i;
	for (i = 0; i < 7; i++) {
		signed_char <<= 1;
		signed_char++;
	}
	TERMINAL2("Largest signed char: %d", signed_char);

	// most negative signed char
	TERMINAL2("Most negative signed char: %d", (signed char)(signed_char + 1));

	// largest unsigned char
	unsigned char unsigned_char = 0;
	TERMINAL2("Largest unsigned char: %u", (char)(unsigned_char - 1));

	// largest signed long
	signed long int signed_long = 1;
	signed_long <<= 31;
	signed_long = ~signed_long;
	TERMINAL1("Largest signed long: %ld", signed_long);

	// most negative signed long
	TERMINAL1("Most negative signed long: %ld", ~signed_long);

	// largest unsigned long
	unsigned long int unsigned_long = ~((long)0);
	TERMINAL1("Largest unsigned long: %lu", unsigned_long);


	/********** Part 2 **********/
	TERMINAL("\n\r** PART 2 *************");

	// integer overflow
	TERMINAL1("Overflow: Largest signed int + 1: %d", signed_int + 1);

	// char overflow
	TERMINAL2("Overflow: Largest signed char + 1: %d", signed_char + 1);

	// long overflow
	TERMINAL1("Overflow: Largest signed long + 1: %d", signed_long + 1);


	/********** Part 3 **********/
	TERMINAL("\n\r** PART 3 *************");

	TERMINAL3("Largest Q16.16 = %f (0x%4x)", ((long)signed_long/65536.0), signed_long);


    return 0;
}
