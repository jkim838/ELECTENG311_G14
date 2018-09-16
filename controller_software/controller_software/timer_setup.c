/*
 * timer_setup.c
 *
 * Created: 14/09/2018 8:53:20 AM
 * Author: Oliver K jkim838 846548800
 *
 * Revision 1.1.5
 *
 * This script file contains definitions for functions related to Timer and PWM.
 *
 */ 

/*** Header Files ***/
#include <avr/io.h>
#include <stdint.h>

/*** Custom Header Files ***/
#include "timer_setup.h"

/*** Custom Macro Definition ***/


void timer0_init(){
	
	/*** OCR0A: Output Compare Register A ***/
	OCR0A = 64;

	/*** TCCR0A: Timer/Counter Control Register A ***/
	/*** Compare Match Output A Mode ***/
	TCCR0A |=   (1 << COM0A1);		// Clear Output Compare Pin (OC0A) on Compare Match (non-inverting), Set OC0A at BOTTOM.
	TCCR0A &=  ~(1 << COM0A0);
	
	/*** Waveform Generation Mode ***/
	TCCR0A |=  (1<< WGM02);			// Fast PWM Mode TOP = 0xff, Update OCR0A at BOTTOM, TOV at 0xff
	TCCR0A |=  (1<< WGM01);
	TCCR0A |=  (1<< WGM00);

	/*** TCCR0B: Timer/Counter Control Reigster B ***/
	/** Force Output Compare A ***/
	//TCCR0B &= ~(1 << FOC0A);		// Force Output Compare A 'off'.
	
	/*** Force Output Compare B ***/
	//TCCR0B &= ~(1 << FOC0B);		// Force Output Compare B 'off'.
	
	/*** Clock Select Bits ***/
	// NOTES: CHANGE PRESCALER VALUE IF WE WANT TO PRODUCE SMALLER COUNT
	TCCR0B |= (1 << CS02);			// Pre-scaler 256
	TCCR0B &= ~(1 << CS01);			// Timer Frequency = 62.5 KHz @Xplained Mini (F_CPU = 16MHz)
	TCCR0B &= ~(1 << CS00);			// Timer Frequency = 31.25 KHz @PCB (F_CPU = 8MHz)
	
	// A match between OCR0A and TCNT0 will call interrupt, or generate waveform output on OC0A pin
	
	/*** TIMSK0: Timer / Counter 0 Mask Register (Actuator) ***/
	/*** Timer / Counter 0 Output Compare Match A Interrupt ***/
	//TIMSK0 |=  (1 << OCIE0A);						// Interrupt when TCNT0 = OCR0A 'On'
	/*** Timer / Counter 0 Output Compare Match B Interrupt ***/
	//TIMSK0 &= ~(1 << OCIE0B);						// Interrupt when TCNT0 = OCR0B 'Off'
	/*** Timer / Counter 0 Overflow Interrupt Enable ***/
	TIMSK0 |=  (1 << TOIE0);						// Interrupt when Overflow On 
	
}

void timer2_init(){
	
	/*** OCR2A: Output Compare Register A ***/
	OCR2A = 64;

	/*** TCCR2A: Timer/Counter Control Register A ***/
	/*** Compare Match Output A Mode ***/
	TCCR2A |=   (1 << COM2A1);		// Clear Output Compare Pin (OC2A) on Compare Match (non-inverting), Set OC2A at BOTTOM.
	TCCR2A &=  ~(1 << COM2A0);
	
	/*** Waveform Generation Mode ***/
	TCCR2A |=  (1<< WGM22);			// Fast PWM Mode TOP = 0xff, Update OCR0A at BOTTOM, TOV at 0xff
	TCCR2A |=  (1<< WGM21);
	TCCR2A |=  (1<< WGM20);
	
	/*** Clock Select Bits ***/
	// NOTES: CHANGE PRESCALER VALUE IF WE WANT TO PRODUCE SMALLER COUNT
	TCCR2B |=  (1 << CS22);			// Pre-scaler 256
	TCCR2B |=  (1 << CS21);			// Timer Frequency = 62.5 KHz @Xplained Mini (F_CPU = 16MHz)
	TCCR2B &= ~(1 << CS20);			// Timer Frequency = 31.25 KHz @PCB (F_CPU = 8MHz)
	
	// A match between OCR0A and TCNT0 will call interrupt, or generate waveform output on OC0A pin
	
	/*** TIMSK0: Timer / Counter 0 Mask Register (Actuator) ***/
	/*** Timer / Counter 0 Output Compare Match A Interrupt ***/
	//TIMSK0 |=  (1 << OCIE0A);						// Interrupt when TCNT0 = OCR0A 'On'
	/*** Timer / Counter 0 Output Compare Match B Interrupt ***/
	//TIMSK0 &= ~(1 << OCIE0B);						// Interrupt when TCNT0 = OCR0B 'Off'
	/*** Timer / Counter 0 Overflow Interrupt Enable ***/
	TIMSK2 |=  (1 << TOIE2);						// Interrupt when Overflow On
	
}
// NOTE
// PWM Period should be 8.2ms approximately at 256 prescaler.
// Fpwm = F_CPU / (N*256) where N is prescaler value.