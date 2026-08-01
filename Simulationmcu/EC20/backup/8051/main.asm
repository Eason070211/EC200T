;==============================================================================
; 8051 PWM Generator with ADC-Controlled Duty Cycle
;
; Hardware:
;   P2.0  - PWM square wave output
;   P1.2  - Analog voltage input (voltage divider)
;
; Function:
;   Reads analog voltage via P1.2 ADC input.
;   Calculates duty cycle: DutyCycle = 1 - (V_div / 5.0)
;   Generates PWM on P2.0 accordingly.
;
;   Examples:
;     V_div = 0.0V -> Duty = 100% (always high)
;     V_div = 2.5V -> Duty =  50%
;     V_div = 5.0V -> Duty =   0% (always low)
;==============================================================================

; Include standard 8051 register definitions
$INCLUDE(reg51.inc)

; Constants
PWM_PERIOD  EQU  256     ; PWM resolution (0-255)

; Variables in internal RAM (DATA)
DUTY_CYCLE  DATA  30H    ; Current duty cycle (0-255)
PWM_TICK    DATA  31H    ; PWM position counter (0-255)

;==============================================================================
; Interrupt Vector Table
;==============================================================================
            ORG   0000H
            LJMP  MAIN          ; Reset vector
            ORG   000BH
            LJMP  TIMER0_ISR    ; Timer 0 interrupt vector

;==============================================================================
; Main Program Initialization
;==============================================================================
            ORG   0100H
MAIN:
            ; Initialize stack pointer
            MOV   SP, #5FH

            ; Initialize variables
            MOV   DUTY_CYCLE, #128   ; Start at 50% duty
            MOV   PWM_TICK, #0

            ; Configure Timer 0: Mode 2 (8-bit auto-reload)
            MOV   TMOD, #02H
            MOV   TH0, #00H          ; Count full 0-255 range
            MOV   TL0, #00H

            ; Enable interrupts
            SETB  ET0                ; IE.1 = 1, enable Timer 0 interrupt
            SETB  EA                 ; IE.7 = 1, global interrupt enable

            ; Start Timer 0
            SETB  TR0                ; TCON.4 = 1

;==============================================================================
; Main Control Loop
;==============================================================================
MAIN_LOOP:
            ; Step 1: Read analog voltage from P1 (P1.2 is ADC input)
            MOV   A, P1              ; Read Port 1

            ; Step 2: Calculate duty cycle: duty = 255 - adc_value
            MOV   R0, A              ; Save ADC value
            MOV   A, #255
            CLR   C                  ; Clear carry for subtraction
            SUBB  A, R0              ; A = 255 - ADC_value

            ; Step 3: Store updated duty cycle
            MOV   DUTY_CYCLE, A

            ; Step 4: Wait ~10ms before next sample
            ACALL DELAY_10MS

            SJMP  MAIN_LOOP          ; Repeat forever

;==============================================================================
; Timer 0 Interrupt Service Routine (interrupt 1)
; Generates PWM waveform on P2.0 using compare method.
;
; Timer 0 configured in Mode 2 (8-bit auto-reload).
; Overflow rate: Fosc/12/256 (e.g., ~3.9kHz @ 12MHz)
;==============================================================================
TIMER0_ISR:
            ; Save registers
            PUSH  ACC
            PUSH  PSW

            ; Advance PWM position counter
            MOV   A, PWM_TICK
            INC   A
            CJNE  A, #PWM_PERIOD, TICK_OK
            MOV   A, #0              ; Wrap around at 256
TICK_OK:
            MOV   PWM_TICK, A

            ; Compare PWM_TICK with DUTY_CYCLE
            ; Set P2.0 high during the first 'duty_cycle' ticks, low for the rest
            MOV   A, DUTY_CYCLE
            SETB  C                  ; Set carry for comparison
            SUBB  A, PWM_TICK        ; A = DUTY_CYCLE - PWM_TICK - 1
            JC    SET_LOW            ; If DUTY_CYCLE <= PWM_TICK, set low

            ; P2.0 = 1 (output high)
SET_HIGH:
            SETB  P2.0
            SJMP  ISR_DONE

            ; P2.0 = 0 (output low)
SET_LOW:
            CLR   P2.0

ISR_DONE:
            POP   PSW
            POP   ACC
            RETI

;==============================================================================
; Delay ~10ms
; Assumes ~12MHz clock: 12MHz/12 = 1MHz instruction cycle = 1us per cycle
; Inner loop: 2 cycles per iteration * 100 = 200 cycles
; Outer loop: 200 * 50 = 10000 cycles = 10ms
;==============================================================================
DELAY_10MS:
            MOV   R6, #50            ; Outer loop count
DELAY_OUTER:
            MOV   R7, #100           ; Inner loop count
DELAY_INNER:
            DJNZ  R7, DELAY_INNER    ; 2 cycles per iteration
            DJNZ  R6, DELAY_OUTER    ; Loop control
            RET

            END
