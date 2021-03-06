;*******************************************************************************
;   Lab 5b - traffic.asm
;
;   Description:  1. Turn the large green LED and small red LED on and
;                    delay 20 seconds while checking for orange LED.
;                    (If orange LED is on and 10 seconds has expired, immediately
;                    skip to next step.)
;                 2. Turn large green LED off and yellow LED on for 5 seconds.
;                 3. Turn yellow LED off and large red LED on.
;                 4. If orange LED is on, turn small red LED off and small green
;                    LED on.  After 5 seconds, toggle small green LED on and off
;                    for 6 seconds at 1 second intervals.  Finish by toggling
;                    small green LED on and off for 4 seconds at 1/5 second
;                    intervals.
;                    Else, turn large red LED on for 5 seconds.
;                 5. Repeat the stoplight cycle.
;
;   I certify this to be my source code and not obtained from any student, past
;   or current.
;
;*******************************************************************************
;                            MSP430F2274
;                  .-----------------------------.
;            SW1-->|P1.0^                    P2.0|<->LCD_DB0
;            SW2-->|P1.1^                    P2.1|<->LCD_DB1
;            SW3-->|P1.2^                    P2.2|<->LCD_DB2
;            SW4-->|P1.3^                    P2.3|<->LCD_DB3
;       ADXL_INT-->|P1.4                     P2.4|<->LCD_DB4
;        AUX INT-->|P1.5                     P2.5|<->LCD_DB5
;        SERVO_1<--|P1.6 (TA1)               P2.6|<->LCD_DB6
;        SERVO_2<--|P1.7 (TA2)               P2.7|<->LCD_DB7
;                  |                             |
;         LCD_A0<--|P3.0                     P4.0|-->LED_1 (Green)
;        i2c_SDA<->|P3.1 (UCB0SDA)     (TB1) P4.1|-->LED_2 (Orange) / SERVO_3
;        i2c_SCL<--|P3.2 (UCB0SCL)     (TB2) P4.2|-->LED_3 (Yellow) / SERVO_4
;         LCD_RW<--|P3.3                     P4.3|-->LED_4 (Red)
;   TX/LED_5 (G)<--|P3.4 (UCA0TXD)     (TB1) P4.4|-->LCD_BL
;             RX-->|P3.5 (UCA0RXD)     (TB2) P4.5|-->SPEAKER
;           RPOT-->|P3.6 (A6)          (A15) P4.6|-->LED 6 (R)
;           LPOT-->|P3.7 (A7)                P4.7|-->LCD_E
;                  '-----------------------------'
;
;*******************************************************************************
;*******************************************************************************
            .cdecls  C,LIST,"msp430.h"      ; MSP430
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

TENTH_SECOND      	.equ    0x6a00			; small subrouting count
GREEN_STATE			.equ	0x01
GREEN_STATE_2		.equ 	0x02
YELLOW_STATE		.equ	0x03
RED_STATE			.equ	0x04
PED_STATE			.equ	0x05
COUNT				.equ    0

;------------------------------------------------------------------------------
            .text                           ; beginning of executable code
            .retain                         ; Override ELF conditional linking
;-------------------------------------------------------------------------------
start:      mov.w   #__STACK_END,SP         ; init stack pointer
            mov.w   #WDTPW+WDTHOLD,&WDTCTL  ; stop WDT
			bis.b  #0x4f,&P4DIR				; set P4.0-3,6 as output
			bis.b  #0x10,&P3DIR				; set P3.4 as output
			bic.b #0xff,&P4OUT				; turn off p4
			bic.b #0xff,&P3OUT				; turn off p3

			bic.b  #0x0f,&P1SEL          	; select GPIO
          	bic.b  #0x0f,&P1DIR          	; configure P1.0 as inputs
          	bis.b  #0x0f,&P1OUT          	; use pull-up
          	bis.b  #0x0f,&P1REN          	; enable pull-up
          	bis.b  #0x0f,&P1IES          	; trigger on high to low transition
          	bis.b  #0x0f,&P1IE           	; P1.0 interrupt enabled
          	bic.b  #0x0f,&P1IFG          	; P1.0 IFG cleared
          	bis.w  #GIE,SR               	; enable general interrupts

mainloop:
			call #green_light
			call #yellow_light
			ORANGE_TEST						; check if the orange led is lit
			jz else							; n

			call #pedestrian				; if
			jmp mainloop
else:
			call #red_light					; else
            jmp mainloop

;------------------------------------------------------------------------------
;	green_light subroutine
;

green_light:
			RED2_ON							; turn on/off appropriate lights
			GREEN2_OFF
			RED_OFF
			YELLOW_OFF
			ORANGE_OFF
			GREEN_ON

			mov.w #GREEN_STATE,r8

			push #0x064						; delay for ten seconds (delay 100)
			call #delay

			ORANGE_TEST						; check if the orange led is lit
			jz second_state							;
			ret

second_state:
			mov.w #GREEN_STATE_2,r8

			push #0x064						; delay for ten seconds (delay 100)
			call #delay

			ret

;------------------------------------------------------------------------------
;	yellow_light subroutine
;

yellow_light:
			RED2_ON							; turn on/off appropriate lights
			GREEN2_OFF
			RED_OFF
			YELLOW_ON
			GREEN_OFF

			mov.w #YELLOW_STATE,r8

			push #0x032						; delay for five seconds (delay 50)
			call #delay
			ret

;------------------------------------------------------------------------------
;	red_light subroutine
;

red_light:
			RED2_ON							; turn on/off appropriate lights
			GREEN2_OFF
			RED_ON
			YELLOW_OFF
			GREEN_OFF

			mov.w #RED_STATE,r8

			push #0x032
			call #delay						; delay for five seconds (delay 50)
			ret

;------------------------------------------------------------------------------
;	pedestrian subroutine
;

pedestrian:
			RED2_OFF						; turn on/off appropriate lights
			GREEN2_ON
			RED_ON
			YELLOW_OFF
			ORANGE_OFF
			GREEN_OFF

			mov.w #PED_STATE,r8

			push #0x032						; delay for five seconds (delay 50)
			call #delay

			GREEN2_TOGGLE
			push #0x06						; toggle 6 times
			push #0x0a						; delay 1 second between each toggle
			call #pedestrian_toggle

			push #0x014						; toggle 20 times
			push #0x02						; delay .2 seconds between each toggle
			call #pedestrian_toggle

			ret

pedestrian_toggle:
			push r11						; callee save
			push r10						; callee save
			mov.w 8(SP), r10				; access parameter 1
			mov.w 6(SP), r11				; access parameter 2
			mov.w 4(SP), 8(SP)				; move return address
			mov.w 2(SP), 6(SP)				; move r11
			mov.w @SP, 4(SP)				; move r10
			add.w #0x04, SP					; set the stack pointer

			inc.w r10
toggle_loop:
			dec.w r10
			jz end_pedestrian_toggle
			GREEN2_TOGGLE
			push r11
			call #delay
			jmp toggle_loop

end_pedestrian_toggle:
			pop r10							; callee save
			pop r11							; callee save
			ret

;------------------------------------------------------------------------------
;	delay subroutine
;

delay:		push r15						; callee save
			push r14						; callee save
			mov.w 6(SP), r14				; access parameter
			mov.w 4(SP), 6(SP)				; move return address
			mov.w 2(SP), 4(SP)				; move r15
			mov.w @SP, 2(SP)				; move r14
			add.w #0x02, SP					; set the stack pointer
			inc.w r14
			jmp big_timing_loop

small_timing_loop:
			dec.w r15
			and.w r15,r15					; waste 1 cycle
			jnz small_timing_loop

big_timing_loop:
			dec.w r14
			jz end_delay
			mov.w #TENTH_SECOND, r15
			inc.w r15
			jmp small_timing_loop

end_delay:
			pop r14							; callee save
			pop r15							; callee save
			ret								; return from #delay

;------------------------------------------------------------------------------
;	Port 1 ISR
;
P1_ISR:
          	bic.b  #0x0f,&P1IFG			; clear P1.0 Interrupt Flag
          	ORANGE_ON

          	sub.w #GREEN_STATE_2,r8		; check for green state 2
          	jz if
			reti
if:
			mov.w #0x01,r14				; end the big timing loop
          	reti

          	.sect  ".int02"				; P1 interrupt vector
          	.word  P1_ISR

;-------------------------------------------------------------------------------
;           Stack Pointer definition
;-------------------------------------------------------------------------------
            .global __STACK_END
            .sect 	.stack

;------------------------------------------------------------------------------
;           Interrupt Vectors
;------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .word   start                   ; start address
            .end
