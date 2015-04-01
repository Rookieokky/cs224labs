//	life.h	03/23/2015
//*******************************************************************************

#ifndef LIFE_H_
#define LIFE_H_

#define myCLOCK			16000000			// 16 Mhz clock
//#define myCLOCK			1200000				// 1.2 Mhz clock
#define CLOCK			_1MHZ

#define SET_BIT(row, col) life[(row)][(col) >> 3] |= bitmask_lookup[(col) & 0x07]
#define CLEAR_BIT(row, col) life[(row)][(col) >> 3] &= ~bitmask_lookup[(col) & 0x07]

#endif /* LIFE_H_ */
