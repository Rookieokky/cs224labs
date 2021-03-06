/*
 * morse2.h
 *
 *  Created on: Mar 19, 2015
 *      Author: BYU Computer Science
 */

#ifndef MORSE2_H_
#define MORSE2_H_

#define myCLOCK         1050000
#define WDT_CTL         WDT_MDLY_0_5
#define WDT_CPI         500
#define WDT_IPS         (myCLOCK/WDT_CPI)
#define STACK           0x0600
#define DEBOUNCE 	 	10

#define END             0
#define DOT             1
#define DASH            2
#define ELEMENT         ((WDT_IPS*240)/1000)

#define BACKLIGHT		1
#define WPM				5


#endif /* MORSE2_H_ */
