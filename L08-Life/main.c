//	life.c	03/27/2015
//
//	Robert Williams
//
//******************************************************************************
//  The Game of Life
//
//  Lab Description:
//
//  The universe of the Game of Life is an infinite two-dimensional orthogonal
//  grid of square cells, each of which is in one of two states, alive or dead.
//  With each new generation, every cell interacts with its eight neighbors,
//  which are the cells horizontally, vertically, or diagonally adjacent
//  according to the following rules:
//
//  1. A live cell stays alive (survives) if it has 2 or 3 live neighbors,
//     otherwise it dies.
//  2. A dead cell comes to life (birth) if it has exactly 3 live neighbors,
//     otherwise it stays dead.
//
//  An initial set of patterns constitutes the seed of the simulation. Each
//  successive generation is created by applying the above rules simultaneously
//  to every cell in the current generation (ie. births and deaths occur
//  simultaneously.)  See http://en.wikipedia.org/wiki/Conway's_Game_of_Life
//
//  Author:    Paul Roper, Brigham Young University
//  Revisions: June 2013   Original code
//             07/12/2013  life_pr, life_cr, life_nr added
//             07/23/2013  generations/seconds added
//             07/29/2013  100 second club check
//             12/12/2013  SWITCHES, display_results, init for port1 & WD
//	           03/24/2014  init_life moved to lifelib.c, 0x80 shift mask
//	                       blinker added, 2x loops
//             03/23/2015  start_generation() added, display_results(void)
//
//  Built with Code Composer Studio Version: 5.5.0.00090
//******************************************************************************
//  Lab hints:
//
//  The life grid (uint8 life[80][10]) is an 80 row x 80 column bit array.  A 0
//  bit is a dead cell while a 1 bit is a live cell.  The outer cells are always
//  dead.  A boolean cell value (0 or non-zero) is referenced by:
//
//         life[row][col >> 3] & (0x80 >> (col & 0x07))
//
//  Each life cell maps to a 2x2 lcd pixel.
//
//                     00       01             08       09
//  life[79][0-9]   00000000 00000000  ...  00000000 00000000 --> life_pr[0-9]
//  life[78][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0 --> life_cr[0-9]
//  life[77][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0 --> life_nr[0-9]
//  life[76][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0         |
//     ...                                                            |
//  life[75-4][0-9]   ...      ...            ...      ...            v
//     ...
//  life[03][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0
//  life[02][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0
//  life[01][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0
//  life[00][0-9]   00000000 00000000  ...  00000000 00000000
//
//  The next generation can be made directly in the life array if the previous
//  cell values are held in the life_pr (previous row), life_cr (current row),
//  and life_nr (next row) arrays and used to count cell neighbors.
//
//  Begin each new row by moving life_cr values to life_pr, life_nr values to
//  life_cr, and loading life_nr with the row-1 life values.  Then for each
//  column, use these saved values in life_pr, life_cr, and life_nr to
//  calculate the number of cell neighbors of the current row and make changes
//  directly in the life array.
//
//  life_pr[0-9] = life_cr[0-9]
//  life_cr[0-9] = life_nr[0-9]
//  life_nr[0-9] = life[row-1][0-9]
//
//******************************************************************************
//******************************************************************************
// includes --------------------------------------------------------------------
#include <stdlib.h>
#include <string.h>
#include "msp430.h"
#include "RBX430-1.h"
#include "RBX430_lcd.h"
#include "life.h"
#include "lifelib.h"

extern volatile uint16 switches;		// debounced switch values
extern const uint16 life_image[];

// global variables ------------------------------------------------------------
uint16	life[NUM_ROWS/4][NUM_COLS/4];	// 80 x 80 life grid
uint16	current_row[(NUM_COLS - 2) / 2];// current row
//uint16	next_row[(NUM_COLS - 2)/2];		// next row
uint16	rowi;							// iterator
uint16 	coli;							// iterator
uint16	rowj;							// parameter
uint16 	colj;							// parameter
uint16	neighbors;						// number of neighbors
uint16	only_neighbors;					// neighbors surrounding bit
uint16 	block_16;
uint16 	position;

void copy_row(uint8* source, uint8* dst) {
	uint8 i;
	for (i = 0; i < NUM_COLS/8; ++i) {
		dst[i] = source[i];
	}
}

void clear_row(uint8* row) {
	uint8 i;
	for (i = 0; i < NUM_COLS/8; ++i) {
		row[i] = 0x00;
	}
}

int isdigit(uint8 character) {
	if (character - '0' >= 0 && character - '0' <= 9) return 1;
	return 0;
}

uint16 read_number(const uint8* string) {
	uint16 number = 0;
	while (isdigit(*string)) {
		number = number * 10 + (*string - '0');
		*string++;
	}
	return number;
}

void set_pixel(uint16 row, uint16 col) {
	// 0x0001 <<
	// ((row / 2) * 2 + col / 2) * 4
	// 				+
	// (row % 2 * 2) + (col % 2)
	SET_BIT(row, col);
	lcd_point((col) << 1, (row) << 1, 7);
}

//------------------------------------------------------------------------------
//	draw RLE pattern -----------------------------------------------------------
void draw_rle_pattern(int row, int col, const uint8* string)
{
	uint16 x = 0;
	uint16 y = 0;
	while (*string != ',') {
		if (isdigit(*string) && x == 0) x = read_number(string);
		++string;
	}

	while (*string != '\n') {
		if (isdigit(*string) && y == 0) y = read_number(string);
		++string;
	}
	++string;

	uint16 current_row = row + y - 1;
	uint16 initial_col = col - 1;
	uint16 current_col = 0;
	uint16 repeat = 0;
	while (*string != 0) {
		if (isdigit(*string)) {
			repeat = read_number(string);
			while (isdigit(*string)) { // skip past the numbers
				++string;
			}
			if (*string == 'b') {
				current_col += repeat;
			} else if (*string == 'o') {
				for (coli = 0; coli < repeat; ++coli) {
					set_pixel(current_row - current_col / (x + 1), current_col % x + initial_col);
					++current_col;
				}
			} else if (*string == '$') {
				current_row -= current_col / (x + 1);
				current_row -= repeat;
				current_col = 0;
			}
		} else if (*string == '$') {
			current_row -= current_col / (x + 1);
			--current_row;
			current_col = 0;
		} else if (*string == 'o') {
			set_pixel(current_row - current_col / (x + 1), current_col % x + initial_col);
			++current_col;
		} else if (*string == 'b') {
			++current_col;
		} else if (*string == '!') {
			break;
		}

		++string;
	}

	return;
} // end draw_rle_pattern


void count_neighbors() {
	neighbors = 0;
	if (position == 1) only_neighbors = block_16 & 0x1357;
	else if (position == 2) only_neighbors = block_16 & 0x32ba;
	else if (position == 3) only_neighbors = block_16 & 0x5d4c;
	else only_neighbors = block_16 & 0xeac8;

	while (only_neighbors && neighbors < 5) {
		neighbors += only_neighbors & 0x0001;
		only_neighbors >>= 1;
	}

	if (neighbors) {
		volatile uint16 test = neighbors;
		test = neighbors;
	}
}

void do_bit() {
	count_neighbors();
	only_neighbors = TEST_BIT(rowj, colj); // using unused variable
	if (neighbors == 3 && only_neighbors == 0) {
		SET_BIT(rowj, colj);
		lcd_point((colj) << 1, (rowj) << 1, 7);
	} else if (neighbors < 2 && neighbors > 3 && only_neighbors) {
		CLEAR_BIT(rowj, colj);
		lcd_point((colj) << 1, (rowj) << 1, 6);
	}
}

void do_row() {
	for (coli = 0; coli < (NUM_ROWS - 2)/2; ++coli) {
		block_16 = current_row[coli];
		if (block_16) {
			volatile uint16 test = block_16;
			volatile uint16 row = rowi;
			position = 1;
			rowj = rowi - 1;
			colj = (coli << 1) + 1;
			do_bit();
			++position;
			++colj;
			do_bit();
			++position;
			++rowj;
			--colj;
			do_bit();
			++position;
			++colj;
			do_bit();
		}
	}
}

//------------------------------------------------------------------------------
// main ------------------------------------------------------------------------
void main(void)
{
	RBX430_init(CLOCK);					// init board
	ERROR2(lcd_init());					// init LCD
	//lcd_volume(376);					// ***increase LCD brightness(if necessary)***
	watchdog_init();					// init watchdog
	port1_init();						// init P1.0-3 switches
	__bis_SR_register(GIE);				// enable interrupts

	while (1)							// new pattern seed
	{
		// setup beginning life generation
		if (switches == 1) {
			init_life(LIFE);
		} else if (switches == 2) {
			init_life(BIRD);
		} else if (switches == 4) {
			init_life(BOMB);
		} else if (switches == 8) {
			init_life(YOURS);
		} else {
			continue;
		}

		while (1) {						// next generation

			rowi = 79;
			for (coli = 0; coli < (NUM_ROWS - 2)/2; ++coli) {
				if (coli % 2 == 0) {
					// even
					current_row[coli] = life[rowi >> 2][coli];
//					next_row[coli] = (life[rowi][coli] >> 8) + (life[1][coli] << 8);
				} else {
					// odd
					current_row[coli] = ((life[rowi >> 4][(coli >> 1)] & 0xF0F0) >> 4) + ((life[rowi >> 4][(coli >> 1) + 1] & 0x0F0F) << 4);
//					next_row[coli] = ((life[rowi][(coli >> 1)] & 0xF000) >> 12) + ((life[rowi][(coli >> 1) + 1] & 0x0F00) >> 4)
//							+ ((life[rowi - 1][(coli >> 1)]) & 0x00F0 << 4) + ((life[rowi - 1][(coli >> 1) + 1] & 0x000F) << 12);
				}
			}

			do_row();

//			clear_row(life_pr); // clear previous row
//			for (row = NUM_ROWS - 2; row > 0; --row) {
//				copy_row(life[row], life_cr);
//				copy_row(life[row - 1], life_nr);
//				for (col = NUM_COLS - 2; col > 0; --col) {
//					neighbors = living_neighbors(col);
//					alive = (life_cr[(col) >> 3] & (0x80 >> ((col) & 0x07)) ? 1 : 0);
//
//					if (alive) {
//						if (neighbors < 2 || neighbors > 3) {
//							life[(row)][(col) >> 3] &= ~(0x80 >> ((col) & 0x07)); // death
//							lcd_point((col) << 1, (row) << 1, 6);
//						}
//					} else {
//						if (neighbors == 3) {
//							life[(row)][(col) >> 3] |= (0x80 >> ((col) & 0x07)); // birth
//							lcd_point((col) << 1, (row) << 1, 7);
//						}
//					}
//				}
//				copy_row(life_cr, life_pr);
//			}

			// display life generation and generations/second on LCD
			if (display_results()) break;
			if (switches) break;
		}
	}
} // end main()
