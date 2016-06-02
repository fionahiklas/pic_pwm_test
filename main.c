/*
 * File:   main.c
 * Author: Fiona Bianchi
 *
 * Based on the code found here: http://burningsmell.org/pic16f628/src/0026-pwm.c
 * Hacked around quite a bit based on the hello world program: https://github.com/fionahiklas/pic_hello_world
 *  
 */


#include <xc.h>
#include <pic16f628a.h>

// CONFIG
#pragma config FOSC = INTOSCIO  // Oscillator Selection bits (INTOSC oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)

// Don't really need this for this program, in fact it should be off 
// otherwise it'll keep restarting your code!
#pragma config WDTE = ON        // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // RA5/MCLR/VPP Pin Function Select bit (RA5/MCLR/VPP pin function is MCLR)
#pragma config BOREN = ON       // Brown-out Detect Enable bit (BOD enabled)

// Make damn sure that this is OFF otherwise your PIC will go mental if you 
// even wave your hand near it or the circuit!  Do NOT waste the hours I did 
// on trying to 'debug' weird glitches and pauses
#pragma config LVP = OFF         // Low-Voltage Programming Enable bit (RB4/PGM pin has PGM function, low-voltage programming enabled)
#pragma config CPD = OFF        // Data EE Memory Code Protection bit (Data memory code protection off)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)


/**
 * Shows operation of the PWM module.
 *
 * Sets up the PWM module to produce output at about 300Hz on pin B3.
 * Meanwhile, timer0 interrupts periodically fire, altering
 * the value of the CCPR1L register and hence altering the
 * duty cycle of the resulting waveform from 0.01%, up to
 * 99.9%, and back down to 0.01% .
 *
 * If you hook a LED up to this pin, it should seem to dim and fade.
 */

static short int dir;

static void interrupt interruptRoutine()
{
	if(T0IF)	// Did we get a timer0 interrupt?
	{
		T0IF=0;

		if(dir)	// count up
		{
			CCPR1L++;
			if(CCPR1L == 0xff)
				dir=0;
		}
		else
		{
			CCPR1L--;
			if(CCPR1L == 0x00)
				dir=1;
		}
	}
}

void main(void)
{
	dir=0;
#ifdef __16f628a	// Only compile this section for PIC16f628a
	CMCON = 0x07;	// Disable comparators.  NEEDED FOR NORMAL PORTA BEHAVIOR ON PIC16f628a!
#endif
	TRISB = 0xf7;	// Set port B as all inputs except B3
    TRISA = 0xff;	// Set PORTA as all inputs

	PR2=0xff;	// Set PWM period
	CCPR1L=0x80;	// Set PWM duty cycle
	CCP1CON=0x04|0x08;	// Set PWM mode
	CCP1X=1;	// Set one of the LSB bits.
			// It took me a while to realize the point of 
			// these since they're so inconvenient to use.
			// Set one but only one of these, and your PWM
			// output will never be able to stall -- values 
			// of 0 won't disable it, values of 255 won't pin it.

	T2CON=0x00;
	T2CKPS0=1;	// Set timer 2 prescaler to 1:16.
	T2CKPS1=1;	// These bits are in T2CON.
	TMR2ON=1;	// Enable timer 2.


	// Set up timer0 interrupt
	T0CS=0;	// Internal clock source
	PSA=0;	// Assign prescaler to timer0
	PS2=0;	PS1=1;	PS0=0;
	INTCON=0;
	GIE=1;
	T0IE=1;
	TMR0=0;

	while(1);	// Loop forever
}