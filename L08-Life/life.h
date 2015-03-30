//	life.h	03/23/2015
//*******************************************************************************

#ifndef LIFE_H_
#define LIFE_H_

#define myCLOCK			1200000			// 1.2 Mhz clock
#define CLOCK			_1MHZ

#define SET_BIT(row, col) life[(row) >> 2][(col) >> 4] |= ((uint16)0x0001 << (((((row >> 1) << 1) + (col >> 1)) << 2) + ((row % 2) << 1 + (col % 2))))
#define CLEAR_BIT(row, col) life[(row) >> 2][(col) >> 4] &= ~((uint16)0x0001 << (((((row >> 1) << 1) + (col >> 1)) << 2) + ((row % 2) << 1 + (col % 2))))
#define TEST_BIT(row, col) life[(row) >> 2][(col) >> 4] & ((uint16)0x0001 << (((((row >> 1) << 1) + (col >> 1)) << 2) + ((row % 2) << 1 + (col % 2))))

#endif /* LIFE_H_ */
