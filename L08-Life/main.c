//	life.c	03/27/2015
//
//	Robert Williams
//  I didn't cheat
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
uint8 life[NUM_ROWS][NUM_COLS/8];		// 80 x 80 life grid
uint8 life_pr[NUM_COLS/8];				// previous row
uint8 life_cr[NUM_COLS/8];				// current row
uint8 bitmask_lookup[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

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

void set_pixel(uint8 row, uint8 col) {
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

	uint16 i;
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
	RBX430_init(CLOCK);					// init board
	ERROR2(lcd_init());					// init LCD
	//lcd_volume(376);					// ***increase LCD brightness(if necessary)***
	watchdog_init();					// init watchdog
	port1_init();						// init P1.0-3 switches
	__bis_SR_register(GIE);				// enable interrupts

	// clock speed
	BCSCTL1 = CALBC1_16MHZ;       // Set range
	DCOCTL = CALDCO_16MHZ;        // Set DCO step and modulation

	while (1)							// new pattern seed
	{
		register uint16 row, col, neighbors, alive, col_index1, col_index2, col_index3;
		register uint8 right_three, middle_three, left_three;

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

			// clear previous row
			life_pr[0] = 0;
			life_pr[1] = 0;
			life_pr[2] = 0;
			life_pr[3] = 0;
			life_pr[4] = 0;
			life_pr[5] = 0;
			life_pr[6] = 0;
			life_pr[7] = 0;
			life_pr[8] = 0;
			life_pr[9] = 0;

			row = NUM_ROWS - 2;
			while (row != 0) {

				life_cr[0] = life[row][0];
				life_cr[1] = life[row][1];
				life_cr[2] = life[row][2];
				life_cr[3] = life[row][3];
				life_cr[4] = life[row][4];
				life_cr[5] = life[row][5];
				life_cr[6] = life[row][6];
				life_cr[7] = life[row][7];
				life_cr[8] = life[row][8];
				life_cr[9] = life[row][9];

				right_three = 0;
				middle_three = 0;
				left_three = 0;
				left_three += life_pr[9] & 0x02;
				left_three += life_cr[9] & 0x02;
				left_three += life[row - 1][9] & 0x02;
				left_three >>= 1;
				neighbors = left_three;
				alive = 0;

				col = NUM_COLS - 2;
				while (col != 0) {
					neighbors -= right_three;
					if (alive) ++neighbors;
					right_three = middle_three;
					middle_three = left_three;
					left_three = 0;

					col_index1 = col - 1;
					col_index2 = col_index1 >> 3;
					col_index3 = col_index1 & 0x07;

					if (life_pr[col_index2]) left_three += (life_pr[col_index2] & bitmask_lookup[col_index3] ? 1 : 0);
					if (life_cr[col_index2]) left_three += (life_cr[col_index2] & bitmask_lookup[col_index3] ? 1 : 0);
					if (life[row - 1][col_index2]) left_three += (life[row - 1][col_index2] & bitmask_lookup[col_index3] ? 1 : 0);

					alive = (life_cr[(col) >> 3] & bitmask_lookup[col & 0x07] ? 1 : 0);
					if (alive) --neighbors;
					if (left_three) neighbors += left_three;

					if (alive && ((neighbors & 0xFC) || neighbors == 1 || neighbors == 0)) CLEAR_BIT(row, col), lcd_point((col) << 1, (row) << 1, 6);
					else if (alive == 0 && neighbors == 3) SET_BIT(row, col), lcd_point((col) << 1, (row) << 1, 7);
					--col;
				}

				life_pr[0] = life_cr[0];
				life_pr[1] = life_cr[1];
				life_pr[2] = life_cr[2];
				life_pr[3] = life_cr[3];
				life_pr[4] = life_cr[4];
				life_pr[5] = life_cr[5];
				life_pr[6] = life_cr[6];
				life_pr[7] = life_cr[7];
				life_pr[8] = life_cr[8];
				life_pr[9] = life_cr[9];

				--row;
			}

			// display life generation and generations/second on LCD
			if (display_results()) break;
			if (switches) break;
		}
	}
} // end main()
