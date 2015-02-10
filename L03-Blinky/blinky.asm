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

SMALL_COUNT      	.equ    0x7000                       ; delay count
FLASH_COUNT			.equ	0x0a
BIG_COUNT			.equ	0x064

;------------------------------------------------------------------------------
            .text                           ; beginning of executable code
;------------------------------------------------------------------------------
start:      mov.w   #0x0280,SP              ; init stack pointer
            mov.w   #WDTPW|WDTHOLD,&WDTCTL  ; stop WDT
            bis.b   #0x01,&P1DIR            ; set P1.0 as output
            mov.w	#0x01,r5				; led status

toggle_led:
			xor.b   #0x01,&P1OUT            ; toggle LED
			xor.w	#0x01,r5				; toggle status
			mov.w	#FLASH_COUNT,r14
			cmp.w	#0x0,r5
			jne		big_timing_loop
			mov.w	#BIG_COUNT,r14
			jmp		big_timing_loop

small_timing_loop:
			sub.w 	#0x01,r15				; subtract 1 from r15
			and.w	r15,r15					; waste 1 cycle
			jnz small_timing_loop			; redo loop if r15 != 0
			jmp big_timing_loop

big_timing_loop:
			sub.w 	#0x01,r14				; subtract 1 from r14
			mov.w	#SMALL_COUNT,r15
			jnz small_timing_loop			; redo loop if r14 != 0
			jmp toggle_led					; toggle the led

;------------------------------------------------------------------------------
;           Interrupt Vectors
;------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .word   start                   ; start address
            .end
