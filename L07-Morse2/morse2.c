#include "msp430.h"          // .cdecls C,"msp430.h"
#include "morse2.h"
#include <stdlib.h>
#include <ctype.h>
#include "RBX430_lcd.h"

extern void doDot(void);
extern void doDash(void);
extern void beep(int);
extern void delay(int);
extern void letter(char*);
extern char* letters[];
extern char* numbers[];
extern int beep_cnt;
extern int delay_cnt;
extern int second_cnt;
extern int beep_enable;
extern int debounce_cnt;

void init_display(char*);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    WDTCTL = WDT_CTL;           // mov.w   #WDT_CTL,&WDTCTL        ; set WD timer interval
    IE1 = WDTIE;                // mov.b   #WDTIE,&IE1             ; enable WDT interrupt
    P4DIR = 0x69;               // bis.b   #0x69,&P4DIR            ; set P4 as output
    P3DIR = 0x10;               // bis.b   #0x10,&P3DIR            ; set P3 as output
    P4OUT &= 0;                 // bic.b   #0xff,&P4OUT            ; turn off p4
    P3OUT &= 0;                 // bic.b   #0xff,&P3OUT            ; turn off p3
    beep_cnt = 0;               // clr.w   &beep_cnt               ; clear counters
    delay_cnt = 0;              // clr.w   &delay_cnt
    _BIS_SR(GIE);               // bis.w   #GIE,SR                 ; enable interrupts
    second_cnt = 1;             // mov.w   #1, &second_cnt         ; initialize second counter
    beep_enable = 1;            // mov.w   #1, &beep_enable        ; initialize beep on

    // initialize switches
    P1SEL &= 0xf0;              // bic.b   #0x0f,&P1SEL            ; RBX430-1 push buttons
    P1DIR &= 0xf0;              // bic.b   #0x0f,&P1DIR            ; Configure P1.0-3 as Inputs
    P1OUT |= 0x0f;              // bis.b   #0x0f,&P1OUT            ; pull-ups
    P1IES |= 0x0f;              // bis.b   #0x0f,&P1IES            ; h to l
    P1REN |= 0x0f;              // bis.b   #0x0f,&P1REN            ; enable pull-ups
    P1IE |= 0x0f;               // bis.b   #0x0f,&P1IE             ; enable switch interrupts
    debounce_cnt = 0;           // mov.w   #0, &debounce_cnt       ; initialize debounce count for switch 1



    // main loop
    while(1) {                                                      // loop:
        char current_character;
        char* message = "HELLO CS 124 WORLD ";
        char* index = message;                                      // mov.w    #message, r6            ; pointer to message

        // initialize strings to be printed to the lcd
        char character_print[4] = {'\'', 'A', '\'', '\0'};
        char speed[2] = {'0', '\0'};

        init_display(message); // get the lcd display ready

        // message loop
        while (current_character = *index++) {                      // word_loop:
                                                                    // mov.b    @r6+, r7                ; one character
                                                                    // cmp.w    #0, r7                  ; check for end of message
        	// print current character
        	lcd_cursor(70, 80);
        	character_print[1] = current_character;
        	lcd_printf(character_print);

        	// print speed
        	lcd_cursor(40, 40);
        	speed[0] = (char)WPM + '0';
        	lcd_printf(speed);

        	char* morse_code;
        	if (isalpha(current_character)) {
        		morse_code = letters[current_character - 'A'];        // sub.w   #'A', r7                ; remove ascii offset
                                                                      // add.w   r7, r7                  ; make the index a word
                                                                      // mov.w   letters(r7), r7         ; pointer to letter code
        	} else if (isdigit(current_character)) {
        		// number
        		morse_code = numbers[current_character - '0'];        // add.w   #17,r7                  ; change ascii offset to numbers
                                                                      // add.w   r7,r7                   ; make the index a word
                                                                      // mov.w   numbers(r7),r7          ; pointer to number code

        	} else {                                                  // cmp.w   #0x020, r7              ; check for end of word
        		// space                                              //    jz   end_word
        		delay(ELEMENT*4);                                     // mov.w   #ELEMENT*4,r15          ; output space
                                                                      //  call   #delay                  ; delay
        		continue;                                             //   jmp   word_loop
        	}

        	letter(morse_code);                                       //  call   #letter
        	delay(ELEMENT*2);                                         // mov.w   #ELEMENT*2,r15          ; output space
                                                                      // call    #delay                  ; delay
                                                                      //  jmp    word_loop
        }
        delay(ELEMENT*4);                                             // mov.w   #ELEMENT*4,r15          ; output space
                                                                      // call   #delay                  ; delay
                                                                      //  jmp   loop                    ; repeat
    }

	return 0;
}

// Port 1 ISR
#pragma vector=PORT1_VECTOR
__interrupt void Port_1_ISR(void) {									// P1_ISR:
   P1IFG &= ~0x0f;													// bic.b   #0x0f, &P1IFG           ; acknowledge (put hands down)
   debounce_cnt = DEBOUNCE;											// mov.w   #DEBOUNCE, &debounce_cnt; reset debounce count
   return;															// reti
}

void init_display(char* message) {
    lcd_init();
    lcd_backlight(BACKLIGHT);
    lcd_clear();

    char* character_string = "Character:";
    char* message_string = "Message:";
    char* speed_string = "Speed:";
    char* wpm_string = "wpm";

    // current character
    lcd_cursor(1, 80);
    lcd_printf(character_string);

    // message
    lcd_cursor(1, 60);
    lcd_printf(message_string);

    lcd_cursor(50, 60);
    lcd_printf(message);

    // speed
    lcd_cursor(1, 40);
    lcd_printf(speed_string);

	lcd_cursor(60, 40);
	lcd_printf(wpm_string);

}
