//******************************************************************************
//	snake_events.c  (04/06/2015)
//
//  Author:			Paul Roper, Brigham Young University
//  Revisions:		1.0		11/25/2012	RBX430-1
//
//******************************************************************************
//
#include "msp430.h"
#include <stdlib.h>
#include "RBX430-1.h"
#include "RBX430_lcd.h"
#include "snake.h"
#include "snakelib.h"

extern volatile uint16 sys_event;			// pending events

volatile enum MODE game_mode;			// 0=idle, 1=play, 2=next
volatile uint16 score;					// current score
volatile uint16 level;					// current level (1-4)
volatile uint16 direction;				// current move direction
volatile uint16 move_cnt;				// snake speed
volatile uint8	timer;					// game timer

volatile uint8 head;					// head index into snake array
volatile uint8 tail;					// tail index into snake array
SNAKE snake[MAX_SNAKE];					// snake segments

extern const uint16 snake_text_image[];		// snake text image
extern const uint16 snake1_image[];			// snake image

static uint8 move_right(SNAKE* head);		// move snake head right
static uint8 move_up(SNAKE* head);			// move snake head up
static uint8 move_left(SNAKE* head);		// move snake head left
static uint8 move_down(SNAKE* head);		// move snake head down
static void new_snake(uint16 length, uint8 dir);
static void delete_tail(void);
static void add_head(void);
void draw_board(void);

//-- switch #1 event -----------------------------------------------------------
//
void SWITCH_1_event(void)
{
	switch (game_mode)
	{
		case IDLE:
			sys_event |= NEW_GAME;
			break;

		case PLAY:
			if (direction != LEFT)
			{
				if (snake[head].point.x < X_MAX)
				{
					direction = RIGHT;
					sys_event |= MOVE_SNAKE;
				}
			}
			break;

		case NEXT:
			sys_event |= NEXT_LEVEL;
			break;
	}
	return;
} // end SWITCH_1_event


//-- switch #2 event -----------------------------------------------------------
//
void SWITCH_2_event(void)
{
	switch (game_mode) {
		case PLAY:
			if (direction != RIGHT) {
				if (snake[head].point.x > 0)
				{
					direction = LEFT;
					sys_event |= MOVE_SNAKE;
				}
			}
			break;
	}
} // end SWITCH_2_event


//-- switch #3 event -----------------------------------------------------------
//
void SWITCH_3_event(void)
{
	switch (game_mode) {
		case PLAY:
			if (direction != UP) {
				if (snake[head].point.y > 0)
				{
					direction = DOWN;
					sys_event |= MOVE_SNAKE;
				}
			}
			break;
	}
} // end SWITCH_3_event


//-- switch #4 event -----------------------------------------------------------
//
void SWITCH_4_event(void)
{
	switch (game_mode) {
		case PLAY:
			if (direction != DOWN) {
				if (snake[head].point.y < Y_MAX)
				{
					direction = UP;
					sys_event |= MOVE_SNAKE;
				}
			}
			break;
	}
} // end SWITCH_4_event


//-- next level event -----------------------------------------------------------
//
void NEXT_LEVEL_event(void)
{
	sys_event = START_LEVEL;
} // end NEXT_LEVEL_event


//-- update LCD event -----------------------------------------------------------
//
void LCD_UPDATE_event(void)
{
	--timer;
	if (timer == 0) sys_event = END_GAME;

	// white text
	lcd_mode(0x02);

	// score
	lcd_cursor(7, 150);
	lcd_printf("Score%u", score);

	// time
	lcd_cursor(123, 150);
	if (timer < 10) {
		lcd_printf("0:0%u", timer);
	} else if (timer == 60) {
		lcd_printf("1:00", timer);
	} else {
		lcd_printf("0:%u", timer);
	}
} // end LCD_UPDATE_event


//-- end game event -------------------------------------------------------------
//
void END_GAME_event(void)
{
	game_mode = IDLE;
	sys_event = 0;
} // end END_GAME_event


//-- move snake event ----------------------------------------------------------
//
void MOVE_SNAKE_event(void)
{
	if (level > 0)
	{
		add_head();						// add head
		lcd_point(COL(snake[head].point.x), ROW(snake[head].point.y), PENTX);
		//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		//	Add code here to check for collisions...
		if (snake[head].xy == ((0 << 8) + 12)) { beep(); blink();}
		lcd_square(COL(12), ROW(0), 2, 1 + FILL);
		//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		delete_tail();					// delete tail
	}
	return;
} // end MOVE_SNAKE_event


//-- start level event -----------------------------------------------------------
//
void START_LEVEL_event(void)
{
	move_cnt = WDT_MOVE2;				// level 2, speed 2
	lcd_clear();						// clear lcd

	// draw the game board
	draw_board();

	// draw snake
	new_snake(START_SCORE, RIGHT);

	// reset the timer
	timer = TIME_1_LIMIT + 1; // this will be decremented immediately

	sys_event |= LCD_UPDATE;

	game_mode = PLAY;					// start level
	return;
} // end START_LEVEL_event


//-- new game event ------------------------------------------------------------
//
void NEW_GAME_event(void)
{
	lcd_clear();						// clear lcd
	lcd_backlight(1);					// turn on backlight
	lcd_wordImage(snake1_image, (159-60)/2, 60, 1);
	lcd_wordImage(snake_text_image, (159-111)/2, 20, 1);

	new_snake(START_SCORE, RIGHT);

	// initialize game variables
	score = START_SCORE; // this happens automatically but I'll do it anyway
	level = 1;

	// wait for the user to start level 1
	game_mode = NEXT;

	// example foods
//	lcd_diamond(COL(16), ROW(20), 2, 1);	// ***demo only***
//	lcd_star(COL(17), ROW(20), 2, 1);
//	lcd_circle(COL(18), ROW(20), 2, 1);
//	lcd_square(COL(19), ROW(20), 2, 1);
//	lcd_triangle(COL(20), ROW(20), 2, 1);
	return;
} // end NEW_GAME_event


//-- new snake -----------------------------------------------------------------
//
void new_snake(uint16 length, uint8 dir)
{
	int i;
	head = 0;
	tail = 0;
	snake[head].point.x = 0;
	snake[head].point.y = 0;
	direction = dir;

	// build snake
	score = length;
	for (i = score - 1; i > 0; --i)
	{
		add_head();
	}
	return;
} // end new_snake


//-- delete_tail  --------------------------------------------------------------
//
void delete_tail(void)
{
	lcd_point(COL(snake[tail].point.x), ROW(snake[tail].point.y), PENTX_OFF);
	tail = (tail + 1) & (~MAX_SNAKE);
} // end delete_tail


//-- add_head  -----------------------------------------------------------------
//
void add_head(void)
{
	static uint8 (*mFuncs[])(SNAKE*) =	// move head function pointers
	             { move_right, move_up, move_left, move_down };
	uint8 new_head = (head + 1) & (~MAX_SNAKE);
	snake[new_head] = snake[head];		// set new head to previous head
	head = new_head;
	// iterate until valid move
	while ((*mFuncs[direction])(&snake[head]));
} // end add_head


//-- move snake head right -----------------------------------------------------
//
uint8 move_right(SNAKE* head)
{
	if ((head->point.x + 1) < X_MAX)		// room to move right?
	{
		++(head->point.x);					// y, move right
		return FALSE;
	}
	if (level != 2)							// n, right fence
	{
		if (level > 2) sys_event = END_GAME;
		head->point.x = 0;					// wrap around
		return FALSE;
	}
	if (head->point.y) direction = DOWN;	// move up/down
	else direction = UP;
	return TRUE;
} // end move_right


//-- move snake head up --------------------------------------------------------
//
uint8 move_up(SNAKE* head)
{
	if ((head->point.y + 1) < Y_MAX)
	{
		++(head->point.y);					// move up
		return FALSE;
	}
	if (level != 2)							// top fence
	{
		if (level > 2) sys_event = END_GAME;
		head->point.y = 0;					// wrap around
		return FALSE;
	}
	if (head->point.x) direction = LEFT;	// move left/right
	else direction = RIGHT;
	return TRUE;
} // end move_up


//-- move snake head left ------------------------------------------------------
//
uint8 move_left(SNAKE* head)
{
	if (head->point.x)
	{
		--(head->point.x);					// move left
		return FALSE;
	}
	if (level != 2)							// left fence
	{
		if (level > 2) sys_event = END_GAME;
		head->point.x = X_MAX - 1;			// wrap around
		return FALSE;
	}
	if (head->point.y) direction = DOWN;	// move down/up
	else direction = UP;
	return TRUE;
} // end move_left


//-- move snake head down ------------------------------------------------------
//
uint8 move_down(SNAKE* head)
{
	if (head->point.y)
	{
		--(head->point.y);					// move down
		return FALSE;
	}
	if (level != 2)							// bottom fence
	{
		if (level > 2) sys_event = END_GAME;
		head->point.y = Y_MAX - 1;			// wrap around
		return FALSE;
	}
	if (head->point.x) direction = LEFT;	// move left/right
	else direction = RIGHT;
	return TRUE;
} // end move_down

//-- draw the board ------------------------------------------------------------
//
void draw_board(void) {
	lcd_rectangle(0, 0, 159, 10, FILL + 1); // bottom
	lcd_rectangle(0, 149, 159, 10, FILL + 1); // top
	lcd_rectangle(0, 0, 7, 159, FILL + 1); // left
	lcd_rectangle(152, 0, 7, 159, FILL + 1); // right

	// white text
	lcd_mode(0x02);

	// button labels
	lcd_cursor(7, 1);
	lcd_printf("UP");
	lcd_cursor(45, 1);
	lcd_printf("DOWN");
	lcd_cursor(87, 1);
	lcd_printf("LEFT");
	lcd_cursor(123, 1);
	lcd_printf("RIGHT");

	// level
	lcd_cursor(70, 150);
	lcd_printf("Level%u", level);
}
