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
uint16	life[NUM_ROWS][NUM_COLS/16];	// 80 x 80 life grid

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

void set_pixel(uint16 x, uint16 y) {
	// 0x0001 <<
	// ((row / 2) * 2 + col / 2) * 4
	// 				+
	// (row % 2 * 2) + (col % 2)
	SET_BIT(x, y);
	lcd_point((y) << 1, (x) << 1, 7);
}

//------------------------------------------------------------------------------
//	draw RLE pattern -----------------------------------------------------------
void draw_rle_pattern(int start_row, int start_col, const uint8* string)
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

	uint8 i;
	uint16 current_row = start_row + y - 1;
	uint16 initial_col = start_col - 1;
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
				for (i = 0; i < repeat; ++i) {
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

//------------------------------------------------------------------------------
// main ------------------------------------------------------------------------

void main(void)
{
	uint8 row;
	uint8 col;
	uint16 position;
	uint16 top_row[NUM_COLS/16];
	uint16 middle_row[NUM_COLS/16];
	uint16 bottom_row;
	uint16 new_row;
	uint8 left_three;
	uint8 middle_three;
	uint8 right_three;
	uint8 bit_value;
	uint16 neighbors;
	uint16 col_index;

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
			middle_row[0] = 0;
			middle_row[1] = 0;
			middle_row[2] = 0;
			middle_row[3] = 0;
			middle_row[4] = 0;

			row = 78;
			while (row > 0) {
				top_row[0] = middle_row[0];
				top_row[1] = middle_row[1];
				top_row[2] = middle_row[2];
				top_row[3] = middle_row[3];
				top_row[4] = middle_row[4];
				middle_row[0] = life[row][0];
				middle_row[0] = life[row][1];
				middle_row[0] = life[row][2];
				middle_row[0] = life[row][3];
				middle_row[0] = life[row][4];
				bottom_row = life[row - 1][4];
				col_index = 4;

				bit_value = 0;
				new_row = 0;
				right_three = 0;
				middle_three = 0;
				left_three = 0;
				left_three += (top_row[col_index] & 0x4000) ? 1 : 0;
				left_three += (middle_row[col_index] & 0x4000) ? 1 : 0;
				left_three += (bottom_row & 0x4000) ? 1 : 0;

				col = 78;
				while (col > 0) {
					position = 0x0001 << (col & 0x0F); // col % 16

					bit_value = (middle_row[col_index] & position) ? 1 : 0;

					right_three = middle_three;
					middle_three = left_three;

					if (position) {
						position >>= 1; // decrement position
					} else {
						bottom_row = life[row][col_index - 1];
						position = 0x8000;
					}
					left_three = 0;
					left_three += (top_row[col_index] & position) ? 1 : 0;
					left_three += (middle_row[col_index] & position) ? 1 : 0;
					left_three += (bottom_row & position) ? 1 : 0;

					neighbors = left_three + middle_three + right_three - bit_value;

					volatile uint16 whatever = row;
					volatile uint16 something = col;
					volatile uint16 test = neighbors;

					if (neighbors == 3 && bit_value == 0) {
						++new_row;
						lcd_point((row) << 1, (col) << 1, 7); // birth
					} else if (neighbors < 2 && neighbors > 3 && bit_value) {
						lcd_point((row) << 1, (col) << 1, 6); // death
					} else {
						new_row += bit_value;
					}

					new_row <<= 1;
					if (position == 0x8000) { // position would be 0 but it was set to 15
						life[row][col_index] = new_row;
						--col_index;
					}

					--col;
				}
				new_row <<= 1;
				life[row][0] = new_row;

				--row;
			}

			// display life generation and generations/second on LCD
			if (display_results()) break;
			if (switches) break;
		}
	}
} // end main()
