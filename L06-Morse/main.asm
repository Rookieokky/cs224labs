			.title	"morse.asm"
;*******************************************************************************
;     Project:  morse.asm
;      Author:  Robert Williams
;
; Description:  Outputs a message in Morse Code using a LED and a transducer
;               (speaker).  The watchdog is configured as an interval timer.
;               The watchdog interrupt service routine (ISR) toggles the green
;               LED every second and pulse width modulates (PWM) the speaker
;               such that a tone is produced.
;
;	Morse code is composed of dashes and dots:
;
;        1. A dot is equal to an element of time.
;        2. One dash is equal to three dots.
;        3. The space between parts of the letter is equal to one dot.
;        4. The space between two letters is equal to three dots.
;        5. The space between two words is equal to seven dots.
;
;    5 WPM = 60 sec / (5 * 50) elements = 240 milliseconds per element.
;    element = (WDT_IPS * 6 / WPM) / 5
;
;******************************************************************************

; LED commands --------------------------------------------------------------

		.asg   "bis.b #0x08,&P4OUT",RED_ON
		.asg   "bic.b #0x08,&P4OUT",RED_OFF
		.asg   "xor.b #0x08,&P4OUT",RED_TOGGLE
		.asg   "bit.b #0x08,&P4OUT",RED_TEST

		.asg   "bis.b #0x04,&P4OUT",YELLOW_ON
		.asg   "bic.b #0x04,&P4OUT",YELLOW_OFF
		.asg   "xor.b #0x04,&P4OUT",YELLOW_TOGGLE
		.asg   "bit.b #0x04,&P4OUT",YELLOW_TEST

		.asg   "bis.b #0x02,&P4OUT",ORANGE_ON
		.asg   "bic.b #0x02,&P4OUT",ORANGE_OFF
		.asg   "xor.b #0x02,&P4OUT",ORANGE_TOGGLE
		.asg   "bit.b #0x02,&P4OUT",ORANGE_TEST
		.asg   "bit.b #0x02,&P4OUT",ORANGE_TEST

		.asg   "bis.b #0x01,&P4OUT",GREEN_ON
		.asg   "bic.b #0x01,&P4OUT",GREEN_OFF
		.asg   "xor.b #0x01,&P4OUT",GREEN_TOGGLE
		.asg   "bit.b #0x01,&P4OUT",GREEN_TEST

		.asg   "bis.b #0x40,&P4OUT",RED2_ON
		.asg   "bic.b #0x40,&P4OUT",RED2_OFF
		.asg   "xor.b #0x40,&P4OUT",RED2_TOGGLE
		.asg   "bit.b #0x40,&P4OUT",RED2_TEST

		.asg   "bis.b #0x10,&P3OUT",GREEN2_ON
		.asg   "bic.b #0x10,&P3OUT",GREEN2_OFF
		.asg   "xor.b #0x10,&P3OUT",GREEN2_TOGGLE
		.asg   "bit.b #0x10,&P3OUT",GREEN2_TEST

; System equates --------------------------------------------------------------
            .cdecls C,"msp430.h"            ; include c header
myCLOCK     .equ    1200000                 ; 1.2 Mhz clock
WDT_CTL     .equ    WDT_MDLY_0_5            ; WD: Timer, SMCLK, 0.5 ms
WDT_CPI     .equ    500                     ; WDT Clocks Per Interrupt (@1 Mhz)
WDT_IPS     .equ    (myCLOCK/WDT_CPI)       ; WDT Interrupts Per Second
STACK       .equ    0x0600                  ; top of stack

; Morse Code equates ----------------------------------------------------------
END         .equ    0
DOT         .equ    1
DASH        .equ    2
ELEMENT     .equ    ((WDT_IPS*240)/1000)    ; (WDT_IPS * 6 / WPM) / 5

; External references ---------------------------------------------------------
            .ref    numbers                 ; codes for 0-9
            .ref    letters                 ; codes for A-Z
                            
; Global variables ------------------------------------------------------------
            .bss    beep_cnt,2              ; beeper flag
            .bss    delay_cnt,2             ; delay flag
            .bss	second_cnt,2			; one second delay

; Program section -------------------------------------------------------------
            .text                           ; program section
message:    .string "SOS",0               ; PARIS message
            .byte   0
            .align  2                       ; align on word boundary

; power-up reset --------------------------------------------------------------
RESET:      mov.w   #STACK,SP               ; initialize stack pointer
            call    #main_asm               ; call main function
            jmp     $                       ; you should never get here!

; start main function vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv	
main_asm:
			mov.w   #WDT_CTL,&WDTCTL        ; set WD timer interval
            mov.b   #WDTIE,&IE1             ; enable WDT interrupt
            bis.b   #0x60,&P4DIR            ; set P4 as output
			bis.b 	#0x10,&P3DIR			; set P3 as output
			bic.b	#0xff,&P4OUT			; turn off p4
			bic.b	#0xff,&P3OUT			; turn off p3
            clr.w   &beep_cnt				; clear counters
            clr.w   &delay_cnt
            bis.w   #GIE,SR                 ; enable interrupts
            mov.w	#1, &second_cnt			; initialize second counter

; output 'A' in morse code (DOT, DASH, space)
loop:
			mov.w	#message, r6			; pointer to message

word_loop:
			mov.w   #ELEMENT*3,r15          ; output space
             call	#delay                  ; delay

			mov.b	@r6+, r7				; one character
			sub.w	#'A', r7				; remove ascii offset
			add.w	r7, r7					; make the index a word
			mov.w	letters(r7), r7			; pointer to letter code

			cmp.w	#0, r7					; check for end of string
			  jnz	letter_loop

            mov.w   #ELEMENT*7,r15          ; output space
            call    #delay                  ; delay
            jmp     loop                    ; repeat

letter_loop:
			mov.b	@r7+, r8				; get dot, dash, or end
			cmp.b	#DOT, r8				; check for a dot
			  jnz	not_dot
			 call	#doDot
			  jmp	letter_loop
not_dot:
			cmp.b	#DASH, r8				; check for a dash
			  jnz	word_loop
			 call	#doDash
			  jmp	letter_loop

; end main function ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

; start doDot subroutine ------------------------------------------------------
doDot:
			push r15						; callee save

			mov.w   #ELEMENT,r15            ; output DOT
            call    #beep
            mov.w   #ELEMENT,r15            ; delay 1 element
            call    #delay

			pop r15							; callee save
			ret

; start doDash subroutine -----------------------------------------------------
doDash:
			push r15						; callee save

            mov.w   #ELEMENT*3,r15          ; output DASH
            call    #beep
            mov.w   #ELEMENT,r15            ; delay 1 element
            call    #delay

			pop r15							; callee save
			ret

; beep (r15) ticks subroutine -------------------------------------------------
beep:       mov.w   r15,&beep_cnt           ; start beep
			clr.w	&delay_cnt				; clear delay

beep02:     tst.w   &beep_cnt               ; beep finished?
              jne   beep02                  ; n
            ret                             ; y


; delay (r15) ticks subroutine ------------------------------------------------
delay:      mov.w   r15,&delay_cnt          ; start delay
			clr.w	&beep_cnt				; clear beep

delay02:    tst.w   &delay_cnt              ; delay done?
              jne   delay02                 ; n
            ret                             ; y


; Watchdog Timer ISR ----------------------------------------------------------
WDT_ISR:
			tst.w   &beep_cnt               ; beep on?
              jeq   WDT_01                  ; n
            dec.w   &beep_cnt               ; y, decrement count
            RED2_ON
            xor.b   #0x20,&P4OUT            ; beep using 50% PWM
            jmp WDT_02

WDT_01:
			RED2_OFF

WDT_02:
			tst.w   &delay_cnt              ; delay?
              jeq   WDT_03                  ; n
            dec.w   &delay_cnt              ; y, decrement count

WDT_03:
			dec.w	&second_cnt
			  jnz	WDT_END
			GREEN2_TOGGLE
			mov.w	#WDT_IPS, &second_cnt	; re-initialize second counter

WDT_END:
			reti                            ; return from interrupt

; Interrupt Vectors -----------------------------------------------------------
            .sect   ".int10"                ; Watchdog Vector
            .word   WDT_ISR                 ; Watchdog ISR

            .sect   ".reset"                ; PUC Vector
            .word   RESET                   ; RESET ISR
            .end
