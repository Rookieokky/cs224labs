;*******************************************************************************
;   Robert Williams -- CS 224 Lab 03
;	IP belongs to the author :)
;
;   Description: Quickly blink P1.0 every 10 seconds.  Calculate MCLK, CPI, MIPS
;        Author:
;
;             MSP430G5223
;             -----------
;            |       P1.0|-->LED1-RED LED
;            |       P1.3|<--S2
;            |       P1.6|-->LED2-GREEN LED
;
; Show all calculations:
;
;   MCLK = _______ cycles / _______ interval = _______ Mhz
;    CPI = _______ cycles/ _______ instructions = _______ Cycles/Instruction
;   MIPS = MCLK / CPI / 1000000 = _______ MIPS

;
;*******************************************************************************
           .cdecls  C,"msp430.h"            ; MSP430

SMALL_COUNT      	.equ    0xFFFF                       ; delay count
BIG_COUNT			.equ	0x0a

;------------------------------------------------------------------------------
            .text                           ; beginning of executable code
;------------------------------------------------------------------------------
start:      mov.w   #0x0280,SP              ;    init stack pointer
            mov.w   #WDTPW|WDTHOLD,&WDTCTL  ;    stop WDT
            bis.b   #0x01,&P1DIR            ;    set P1.0 as output

toggle_led:
			xor.b   #0x01,&P1OUT            ; toggle LED
			mov.w	#SMALL_COUNT,r15		; put count in r15
			xor.w	r14,r14					; zero r14
			cmp.b	#0x0,&P1OUT
			jne small_timing_loop
			mov.w	#BIG_COUNT,r14
			jmp big_timing_loop

small_timing_loop:
			sub.w 	#0x01,r15				; subtract 1 from r15
			jnz small_timing_loop			; redo loop if r15 != 0
			jmp big_timing_loop

big_timing_loop:
			sub.w 	#0x01,r14				; subtract 1 from r15
			mov.w	#SMALL_COUNT,r15
			jnz small_timing_loop			; redo loop if r15 != 0
			jmp toggle_led					; toggle the led

;------------------------------------------------------------------------------
;           Interrupt Vectors
;------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .word   start                   ; start address
            .end
