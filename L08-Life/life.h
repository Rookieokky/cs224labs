//	life.h	03/23/2015
//*******************************************************************************

#ifndef LIFE_H_
#define LIFE_H_

#define myCLOCK			1200000			// 1.2 Mhz clock
#define CLOCK			_1MHZ

#define SET_BIT(row, col) life[row][col >> 4] |= (0x8000 >> (col & 0x007F))
#define CLEAR_BIT(row, col) life[row][col >> 4] &= ~(0x8000 >> (col & 0x000E))

#endif /* LIFE_H_ */
