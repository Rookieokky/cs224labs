//******************************************************************************
//	snakelib.c  (04/06/2015)
//
//  Author:			Paul Roper, Brigham Young University
//  Revisions:		1.0		04/06/2015	Snake functions
//
//******************************************************************************
//
#include "msp430.h"
#include <stdlib.h>
#include "RBX430-1.h"
#include "RBX430_lcd.h"
#include "snake.h"
#include "snakelib.h"

extern volatile uint16 WDT_cps_cnt;			// WDT count per second
extern volatile uint16 WDT_move_cnt;		// counter to move snake event
extern volatile uint16 WDT_debounce_cnt;	// switch debounce counter
extern volatile uint16 backlight_cnt;		// LCD backlight counter
extern volatile uint32 WDT_delay;			// WDT delay counter
extern volatile uint16 TB0_tone_on;			// tone WDT count


//--init TimerB for PWM sound---------------------------------------------------
int timerB_init(void)
{
	// configure h/w PWM for speaker
	P4SEL |= 0x20;						// P4.5 TB2 output
	TBR = 0;							// reset timer B
	TBCTL = TBSSEL_2 | ID_0 | MC_1;		// SMCLK, /1, UP (no interrupts)
	TBCCTL2 = OUTMOD_3;					// TB2 = set/reset
	return 0;
} // end timerB_init


//--outTone---------------------------------------------------------------------
void outTone(unsigned int tone, unsigned int duration)
{

	if (tone)
	{
		TBCCR0 = tone;
		TBCCR2 = tone >> 1;
		TB0_tone_on = duration;
		while (TB0_tone_on);
		WDT_delay = 10;
	}
	else
	{
		WDT_delay = duration + 10;
	}
	while (WDT_delay);
	return;
} // end outTone


//-- beep ----------------------------------------------------------------------
//
void beep(void)
{
	TBCCR0 = BEEP_COUNT;
	TBCCR2 = BEEP_COUNT >> 1;
	TB0_tone_on = 5;			// turn on buzzer
	return;
} // end beep


//-- blink ---------------------------------------------------------------------
//
void blink(void)
{
	backlight_cnt = BLINK_COUNT;	// turn backlight off
	return;
} // end beep


//--rasberry--------------------------------------------------------------------
void rasberry(void)
{
	outTone(65535, 400);
	return;
} // end rasberry


const TONE Charge[] = {
		{ TONE_F, 50 },				// G
		{ TONE_Bb, 50 },			// C
		{ TONE_D, 50 },				// E
		{ TONE_F1, 100 },			// G
		{ TONE_D, 50 },				// E
		{ TONE_F1, 200 }			// G
};

const TONE imperial_march_notes[] = {
		{ TONE_G, 200 },			// 1
		{ TONE_G, 200 },
		{ TONE_G, 200 },
		{ TONE_Eb, 150 },
		{ TONE_Bb, 50 },

		{ TONE_G, 200 },			// 2
		{ TONE_Eb, 150 },
		{ TONE_Bb, 50 },
		{ TONE_G, 400 },

		{ TONE_D1, 200 },			// 3
		{ TONE_D1, 200 },
		{ TONE_D1, 200 },
		{ TONE_Eb1, 150 },
		{ TONE_Bb, 50 },

		{ TONE_Gb, 200 },			// 4
		{ TONE_Eb, 150 },
		{ TONE_Bb, 50 },
		{ TONE_G, 400 },

		{ TONE_G1, 200 },			// 5
		{ TONE_G, 150 },
		{ TONE_G, 50 },
		{ TONE_G1, 200 },
		{ TONE_Fs1, 150 },
		{ TONE_F1, 50 },

		{ TONE_E1, 50 },			// 6
		{ TONE_Ds1, 50 },
		{ TONE_E1, 100 },
		{ TONE_REST, 100 },
		{ TONE_Gs, 100 },
		{ TONE_Cs1, 200 },
		{ TONE_Bs, 150 },
		{ TONE_B1, 50 },

		{ TONE_Bb, 50 },			// 7
		{ TONE_A, 50 },
		{ TONE_Bb, 100 },
		{ TONE_REST, 100 },
		{ TONE_Eb, 100 },
		{ TONE_Gb, 200 },
		{ TONE_Eb, 150 },
		{ TONE_G, 50 },

		{ TONE_Bb, 200 },			// 8
		{ TONE_G, 150 },
		{ TONE_Bb, 50 },
		{ TONE_D1, 400 },

		{ TONE_G1, 200 },			// 9
		{ TONE_G, 150 },
		{ TONE_G, 50 },
		{ TONE_G1, 200 },
		{ TONE_Fs1, 150 },
		{ TONE_F1, 50 },

		{ TONE_E1, 50 },			// 10
		{ TONE_Ds1, 50 },
		{ TONE_E1, 100 },
		{ TONE_REST, 100 },
		{ TONE_Gs, 100 },
		{ TONE_Cs1, 200 },
		{ TONE_Bs, 150 },
		{ TONE_B1, 50 },

		{ TONE_Bb, 50 },			// 11
		{ TONE_A, 50 },
		{ TONE_Bb, 100 },
		{ TONE_REST, 100 },
		{ TONE_Eb, 100 },
		{ TONE_Gb, 200 },
		{ TONE_Eb, 150 },
		{ TONE_Bb, 50 },

		{ TONE_G, 200 },			// 12
		{ TONE_Eb, 150 },
		{ TONE_Bb, 50 },
		{ TONE_G, 400 }
};

void doDitty(int tones, const TONE* ditty)
{
	int i;
	for (i = 0; i < tones; i++)
	{
		outTone(ditty[i].tone, ditty[i].duration);
	}
}

void imperial_march(void)
{
	doDitty(sizeof(imperial_march_notes) / sizeof(TONE), imperial_march_notes);
}


//--charge!!--------------------------------------------------------------------
void charge(void)
{
//	WDT_led_on = 0;
	doDitty(6, Charge);

//	WDT_led_on = 1;
	return;
} // end charge
