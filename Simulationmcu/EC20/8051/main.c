/******************************************************************************
 * 8051 PWM Generator with ADC-Controlled Duty Cycle
 *
 * Hardware Connections:
 *   P2.0  - PWM square wave output
 *   P1.2  - Analog voltage input (voltage divider)
 *
 * Function:
 *   Reads analog voltage via P1.2 ADC input.
 *   Calculates duty cycle: DutyCycle = 1 - (V_div / 5.0)
 *   Generates PWM on P2.0 accordingly.
 *
 *   Examples:
 *     V_div = 0.0V -> Duty = 100% (always high)
 *     V_div = 2.5V -> Duty =  50%
 *     V_div = 5.0V -> Duty =   0% (always low)
 ******************************************************************************/

#include <reg51.h>

// Pin definitions
sbit PWM_OUT = P2^0;        // P2.0: PWM output pin

// PWM parameters
#define PWM_PERIOD  256      // PWM resolution (8-bit, 0-255)

volatile unsigned char duty_cycle = 128;  // Current duty cycle (0-255)
volatile unsigned int pwm_tick = 0;       // PWM position counter

/******************************************************************************
 * Timer 0 Interrupt Service Routine
 * Generates PWM waveform on P2.0 using compare method.
 *
 * Timer 0 configured in Mode 2 (8-bit auto-reload).
 * Overflow rate: Fosc/12/256 (e.g., ~3.9kHz @ 12MHz)
 ******************************************************************************/
void timer0_isr(void) interrupt 1
{
    // Advance PWM position
    pwm_tick++;
    if (pwm_tick >= PWM_PERIOD) {
        pwm_tick = 0;
    }

    // Set output based on compare
    // High during the first 'duty_cycle' ticks, low for the rest
    if (pwm_tick < duty_cycle) {
        PWM_OUT = 1;
    } else {
        PWM_OUT = 0;
    }
}

/******************************************************************************
 * Initialize Timer 0 for PWM generation
 ******************************************************************************/
void timer_init(void)
{
    TMOD = 0x02;             // Timer 0, Mode 2 (8-bit auto-reload)
    TH0  = 0x00;             // Count full 0-255 range
    TL0  = 0x00;
    ET0  = 1;                // Enable Timer 0 interrupt
    TR0  = 1;                // Start Timer 0
}

/******************************************************************************
 * Read ADC value from P1.2
 *
 * In this simulation environment, P1.2 is configured as an analog input.
 * The voltage on P1.2 (0V-5V) is converted to an 8-bit digital value (0-255).
 *
 * Note: The actual ADC conversion mechanism depends on the simulator.
 * This function reads the port value as interpreted by the simulation.
 ******************************************************************************/
unsigned char read_adc(void)
{
    unsigned char adc_val;

    // Read P1 port - P1.2 carries the digitized analog voltage
    // Mask to get only P1.2, scale to 8-bit range
    // (Actual implementation depends on simulator's ADC model)
    adc_val = P1 & 0xFF;     // Read entire Port 1

    return adc_val;
}

/******************************************************************************
 * Simple blocking delay (approximate)
 ******************************************************************************/
void delay_ms(unsigned int ms)
{
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 125; j++);
}

/******************************************************************************
 * Convert ADC value to PWM duty cycle
 *
 * Formula: duty_cycle = 255 - adc_value
 *
 * Derivation:
 *   V_div  = adc_value * 5.0 / 255.0
 *   Duty   = 1.0 - (V_div / 5.0)
 *          = 1.0 - (adc_value / 255.0)
 *          = (255 - adc_value) / 255.0
 *   In 8-bit: duty_cycle = 255 - adc_value
 ******************************************************************************/
unsigned char adc_to_duty(unsigned char adc_val)
{
    // Clamp: if adc_val > 255, treat as 255
    return 255 - adc_val;
}

/******************************************************************************
 * Main Program
 ******************************************************************************/
void main(void)
{
    unsigned char adc_value;
    unsigned char new_duty;

    // Initialize hardware
    timer_init();

    // Enable global interrupts
    EA = 1;

    // Initial duty cycle: 50% (mid-scale)
    duty_cycle = 128;

    // Main control loop
    while (1)
    {
        // Step 1: Read analog voltage from voltage divider on P1.2
        adc_value = read_adc();

        // Step 2: Calculate duty cycle: duty = 255 - adc_value
        new_duty = adc_to_duty(adc_value);

        // Step 3: Update duty cycle (atomic write to volatile)
        duty_cycle = new_duty;

        // Step 4: Wait before next sample
        delay_ms(10);
    }
}
