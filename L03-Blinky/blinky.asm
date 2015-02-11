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
;   MCLK = 90384 cycles / .1s interval = 1.2 Mhz
;    CPI = 26 cycles/ 15 instructions = 1.73 Cycles/Instruction
;   MIPS = MCLK / CPI / 1000000 = .69 MIPS

;
;*******************************************************************************
           .cdecls  C,"msp430.h"            ; MSP430

SMALL_COUNT      	.equ    0x75b0			; small subrouting count
FLASH_COUNT			.equ	0x03			; count of three subloops per flash
WAIT_COUNT			.equ	0x061			; count of 97 subloops per 10 seconds (plus flash)

;------------------------------------------------------------------------------
            .text                           ; beginning of executable code
;------------------------------------------------------------------------------
start:      mov.w   #0x0280,SP              ; init stack pointer
            mov.w   #WDTPW|WDTHOLD,&WDTCTL  ; stop WDT
            bis.b   #0x01,&P1DIR            ; set P1.0 as output
            xor.w	r5,r5					; led status (length 1, cycles 1)
            bic.b   #0x01,&P1OUT			; make sure led is off (length 2, cycles 4)

toggle_led:
			xor.b   #0x01,&P1OUT            ; toggle LED (length 2, cycles 4)
			xor.w	#0x01,r5				; toggle status (length 1, cycles 1)
			mov.w	#FLASH_COUNT,r14		; (length 2, cycles 2)
			cmp.w	#0x0,r5					; (length 1, cycles 1)
			jne		big_timing_loop			; (length 1, cycles 2)
			mov.w	#WAIT_COUNT,r14			; (length 2, cycles 2)
			jmp		big_timing_loop			; (length 1, cycles 2)

small_timing_loop:
			sub.w 	#0x01,r15				; subtract 1 from r15 (length 1, cycles 1)
			and.w	r15,r15					; waste 1 cycle (length 1, cycles 1)
			jnz small_timing_loop			; redo loop if r15 != 0 (length 1, cycles 2)
			jmp big_timing_loop				; (length 1, cycles 2)

big_timing_loop:
			sub.w 	#0x01,r14				; subtract 1 from r14 (length 1, cycles 1)
			mov.w	#SMALL_COUNT,r15		; (length 2, cycles 2)
			jnz small_timing_loop			; redo loop if r14 != 0 (length 1, cycles 2)
			jmp toggle_led					; toggle the led (length 1, cycles 2)

;------------------------------------------------------------------------------
;           Interrupt Vectors
;------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .word   start                   ; start address
            .end
