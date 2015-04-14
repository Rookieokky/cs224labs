//******************************************************************************
//	snake_events.c  (04/06/2015)
//
//	Robert Williams -- I didn't cheat
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

const TONE Scale[] = {
		{ TONE_C, 50 },
		{ TONE_D, 50 },
		{ TONE_E, 50 },
		{ TONE_F, 50 },
		{ TONE_G, 50 },
		{ TONE_F, 50 },
		{ TONE_E, 50 },
		{ TONE_D, 50 },
		{ TONE_C, 50 },
		{ TONE_E, 50 },
		{ TONE_G, 50 },
		{ TONE_E, 50 },
		{ TONE_C, 50 }
};


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

FOOD *foods = NULL;
uint8 level_foods;
uint8 foods_eaten_on_current_level;
uint8 foods_to_eat_on_current_level;

ROCK rocks[MAX_ROCKS];
uint8 num_rocks;

uint8 win;

extern const uint16 snake_text_image[];		// snake text image
extern const uint16 snake1_image[];			// snake image
extern const uint16 king_snake_image[];		// king snake image

static uint8 move_right(SNAKE* head);		// move snake head right
static uint8 move_up(SNAKE* head);			// move snake head up
static uint8 move_left(SNAKE* head);		// move snake head left
static uint8 move_down(SNAKE* head);		// move snake head down
static void new_snake(uint16 length, uint8 dir);
static void delete_tail(void);
static void add_head(void);
void draw_board(void);
void draw_foods(void);
void draw_rocks(void);
void draw_level_banner(void);
void free_foods(void);
void draw_food(uint8);

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
	if (game_mode != PLAY) return;

	// white text
	lcd_mode(0x02);

	// score
	lcd_cursor(7, 150);
	lcd_printf("Score%u", score);

	// time
	lcd_cursor(128, 150);
	if (timer < 10) {
		lcd_printf("0:0%u", timer);
	} else if (timer == 60) {
		lcd_printf("1:00", timer);
	} else {
		lcd_printf("0:%u", timer);
	}

	if (timer == 0) {
		sys_event = END_GAME;
	}
} // end LCD_UPDATE_event


//-- end game event -------------------------------------------------------------
//
void END_GAME_event(void)
{
	game_mode = IDLE;
	
	if (win) {
		lcd_clear();
		lcd_wordImage(king_snake_image, 20, 20, 1);
		imperial_march();
	} else {
		// print out game over and the final score
		lcd_rectangle(14, 39, 134, 79, FILL + 1);
		// white text
		lcd_mode(0x04);
	
		// game over
		lcd_cursor(19, 89);
		lcd_printf("GAME OVER!");
	
		// score
		lcd_cursor(39, 49);
		lcd_printf("Score %u", score);
	
		// play sound
		rasberry();
	}
} // end END_GAME_event


//-- move snake event ----------------------------------------------------------
//
void MOVE_SNAKE_event(void)
{
	if (level > 0)
	{
		add_head();						// add head
		lcd_point(COL(snake[head].point.x), ROW(snake[head].point.y), PENTX);

		// check for food collision
		uint8 food_collision = 0;
		uint8 i = level_foods;
		do {
			--i;
			if (snake[head].xy == foods[i].xy) {
				food_collision = 1;

				foods_eaten_on_current_level++;
				beep();
				blink();

				// add to the score and make sure it is updated
				score += level;
				sys_event |= LCD_UPDATE;
				++timer; // make sure the timer is not messed up by lcd_update

				// move the food off the grid
				foods[i].point.x = X_MAX;
				foods[i].point.y = Y_MAX;

				// spawn new food on levels three and four
				if (level > 2) draw_foods();

				// add a new food if level 1
				if (level == 1) draw_food(i);

				break;
			}
		} while (i > 0);

		uint8 end_game = 0;

		// check for snake collisions with rocks
		if (level == 1) goto no_rocks; // skip the rocks on level 1
		i = num_rocks;
		do {
			--i;
			if (snake[head].xy == rocks[i].xy) {
				end_game = 1;
			}
		} while (i > 0);
		no_rocks:

		// check for snake collisions with itself
		i = head;
		while (i > tail) {
			i--;
			if (snake[head].xy == snake[i].xy) {
				end_game = 1;
			}
		}

		// snake grows on food eaten
		if (food_collision == 0) delete_tail();

		if (end_game) {
			sys_event = END_GAME;
			return;
		}

		if (foods_eaten_on_current_level >= foods_to_eat_on_current_level) {
			level++;
			if (level == 5) {
				game_mode = IDLE;
				sys_event = END_GAME;
				win = 1;
			} else {
				game_mode = NEXT;
				doDitty(13, Scale);
				draw_level_banner();
				sys_event = LCD_UPDATE; // stop move snake from happening again
			}
		}
	}
	return;
} // end MOVE_SNAKE_event


//-- start level event -----------------------------------------------------------
//
void START_LEVEL_event(void)
{


	lcd_clear();						// clear lcd

	// draw the game board
	draw_board();

	// draw the foods
	switch (level) {
	case 1:
		move_cnt = WDT_MOVE1;
		level_foods = LEVEL_1_FOOD;
		foods_to_eat_on_current_level = LEVEL_1_FOOD;
		timer = TIME_1_LIMIT;
		break;
	case 2:
		move_cnt = WDT_MOVE2;
		level_foods = LEVEL_2_FOOD;
		foods_to_eat_on_current_level = LEVEL_2_FOOD;
		timer = TIME_2_LIMIT;
		break;
	case 3:
		move_cnt = WDT_MOVE3;
		level_foods = 1;
		foods_to_eat_on_current_level = LEVEL_3_FOOD;
		timer = TIME_3_LIMIT;
		break;
	case 4:
		move_cnt = WDT_MOVE4;
		level_foods = 1;
		foods_to_eat_on_current_level = LEVEL_4_FOOD;
		timer = TIME_4_LIMIT;
		break;
	}

	foods_eaten_on_current_level = 0;
	draw_foods();

	// draw the rocks
	if (level == 1) num_rocks = 0;
	else num_rocks = (rand() % (MAX_ROCKS - 2)) + 2; // at least two rocks
	draw_rocks();

	// draw snake
	new_snake(START_SCORE, RIGHT);

	// reset the timer
	timer++; // this will be decremented immediately

	// update the lcd with all the new info
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


	// initialize game variables
	score = 0;
	level = BEGINNING_LEVEL;
	win = 0;

	// wait for the user to start level 1
	game_mode = NEXT;

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
	for (i = length - 1; i > 0; --i)
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

void draw_foods(void) {
	free_foods();
	foods = malloc(sizeof(FOOD) * level_foods);

	uint8 i;
	for (i = 0; i < level_foods; i++) {
		draw_food(i);
	}
}

void draw_food(uint8 index) {
	uint8 x_coordinate, y_coordinate, i;

	start_over:
	x_coordinate = rand() % X_MAX;
	y_coordinate = (rand() % (Y_MAX - 1)) + 1; // no foods on bottom row
	foods[index].point.x = x_coordinate;
	foods[index].point.y = y_coordinate;

	i = num_rocks;
	while (i > 0) {
		i--;
		if (rocks[i].xy == foods[index].xy) goto start_over;
	}

	i = head;
	while (i > tail) {
		i--;
		if (snake[i].xy == foods[index].xy) goto start_over;
	}

	switch (index % 5) {
	case 1:
		lcd_circle(COL(x_coordinate), ROW(y_coordinate), 2, 1);
		break;
	case 2:
		lcd_triangle(COL(x_coordinate), ROW(y_coordinate), 2, 1);
		break;
	case 3:
		lcd_star(COL(x_coordinate), ROW(y_coordinate), 2, 1);
		break;
	case 4:
		lcd_diamond(COL(x_coordinate), ROW(y_coordinate), 2, 1);
		break;
	default:
		lcd_square(COL(x_coordinate), ROW(y_coordinate), 2, 1);
		break;
	}
}

void draw_rocks(void) {
	uint8 i;
	for (i = 0; i < num_rocks; i++) {
		uint8 x_coordinate = rand() % X_MAX;
		uint8 y_coordinate = (rand() % (Y_MAX - 1)) + 1; // no rocks on bottom row
		rocks[i].point.x = x_coordinate;
		rocks[i].point.y = y_coordinate;

		lcd_point(COL(x_coordinate), ROW(y_coordinate), PENTX);
	}
}

void draw_level_banner() {
	// print out game over and the final score
	lcd_rectangle(39, 59, 79, 29, FILL + 1);
	// white text
	lcd_mode(0x04);

	// game over
	lcd_cursor(44, 64);
	lcd_printf("LEVEL%u", level);
}

void free_foods(void) {
	if (foods != NULL) {
		free(foods);
	}
}
