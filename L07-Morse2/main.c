#include "msp430.h"          // .cdecls C,"msp430.h"
#include "morse2.h"
#include <stdlib.h>
#include <ctype.h>

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

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    WDTCTL = WDT_CTL;
    IE1 = WDTIE;
    P4DIR = 0x69;
    P3DIR = 0x10;
    P4OUT &= 0;
    P3OUT &= 0;
    beep_cnt = 0;
    delay_cnt = 0;
//    GIE = SR;
    _BIS_SR(GIE); // set general interrupts
    second_cnt = 1;
    beep_enable = 1;

    // initialize switches
    P1SEL &= 0xf0;
    P1DIR &= 0xf0;
    P1OUT |= 0x0f;
    P1IES |= 0x0f;
    P1REN |= 0x0f;
    P1IE |= 0x0f;
    debounce_cnt = 0;

    // main loop
    while(1) {
        char current_character;
        char* message = "HELLO CS 124 WORLD ";
        char* index = message;
        // message loop
        while (current_character = *index++) {
        	char* morse_code;

        	if (isalpha(current_character)) {
        		morse_code = letters[current_character - 'A'];
        	} else if (isdigit(current_character)) {
        		// number
        		morse_code = numbers[current_character - '0'];
        	} else {
        		// space
        		delay(ELEMENT*4);
        		continue;
        	}

        	letter(morse_code);
        	delay(ELEMENT*2); // space between letters
        }
        delay(ELEMENT*4); // end message
    }

//    ; start main function vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//    			.def 	main_asm
//    main_asm:
//    			mov.w   #WDT_CTL,&WDTCTL        ; set WD timer interval
//                mov.b   #WDTIE,&IE1             ; enable WDT interrupt
//                bis.b   #0x69,&P4DIR            ; set P4 as output
//    			bis.b 	#0x10,&P3DIR			; set P3 as output
//    			bic.b	#0xff,&P4OUT			; turn off p4
//    			bic.b	#0xff,&P3OUT			; turn off p3
//                clr.w   &beep_cnt				; clear counters
//                clr.w   &delay_cnt
//                bis.w   #GIE,SR                 ; enable interrupts
//                mov.w	#1, &second_cnt			; initialize second counter
//                mov.w 	#1, &beep_enable 		; initialize beep on
//
//                ; initialize switches
//                bic.b   #0x0f,&P1SEL 			; RBX430-1 push buttons
//    			bic.b   #0x0f,&P1DIR 			; Configure P1.0-3 as Inputs
//    			bis.b   #0x0f,&P1OUT 			; pull-ups
//    			bis.b   #0x0f,&P1IES 			; h to l
//    			bis.b   #0x0f,&P1REN 			; enable pull-ups
//    			bis.b   #0x0f,&P1IE  			; enable switch interrupts
//    			mov.w 	#0, &debounce_cnt 		; initialize debounce count for switch 1
//
//    loop:
//    			mov.w	#message, r6			; pointer to message
//
//    word_loop:
//    			mov.b	@r6+, r7				; one character
//    			cmp.w	#0x020, r7				; check for end of word
//    			   jz	end_word
//    			cmp.w	#0, r7					; check for end of message
//    			   jz	end_message
//    			sub.w	#'A', r7				; remove ascii offset
//    			   jn	is_number
//    is_letter:
//    			add.w	r7, r7					; make the index a word
//    			mov.w	letters(r7), r7			; pointer to letter code
//    			  jmp	letter_continue
//
//    is_number:
//    			add.w	#17,r7					; change ascii offset to numbers
//    			add.w	r7,r7					; make the index a word
//    			mov.w	numbers(r7),r7			; pointer to number code
//
//
//    letter_continue:
//    			 call	#letter
//
//    			mov.w   #ELEMENT*2,r15          ; output space
//                 call	#delay                  ; delay
//
//    			  jmp	word_loop
//
//    end_word:
//                ;RED_TOGGLE						; debug code
//                mov.w   #ELEMENT*4,r15          ; output space
//                 call   #delay                  ; delay
//                  jmp	word_loop
//
//    end_message:
//    			;GREEN_TOGGLE					; debug code
//                mov.w   #ELEMENT*4,r15          ; output space
//                 call   #delay                  ; delay
//                  jmp   loop                    ; repeat
//
//    ; end main function ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	return 0;
}
