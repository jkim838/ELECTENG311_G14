/*
 * timer_setup.c
 *
 * Created: 14/09/2018 8:53:20 AM
 * Author: Oliver K jkim838 846548800
 *
 * Revision 1.3.0
 *
 * This script file contains definitions for functions related to Timer and PWM.
 *
 */ 

/*** Header Files ***/
#include <avr/io.h>
#include <stdint.h>

/*** Custom Header Files ***/
#include "timer_setup.h"
#include "Macro_Definitions.h"

/*** Custom Macro Definition ***/


void timer0_init(){
	
	/*** TCCR0A: TC0 Control Register A ***/
	// Operate at Normal Timer Mode...
	/*** TCCR0B: TC0 Control Register B ***/
	/** Prescaler Selection **/
	TCCR0B |= (1 << CS01);			// Set prescaler to 64
	TCCR0B |= (1 << CS00);			// Xplained Mini @ F_CPU 16 MHz... F(Timer) = 250KHz
									// IC ATMEGA328P @ F_CPU 8 MHz... F(Timer) = 125KHz
	/*** Compare Match Register ***/
	OCR0A = 0;											
	OCR0B = 64;
	
	/*** TIMSK0: TC0 Interrupt Mask Register ***/
	TIMSK0 |= (1 << OCIE0A);		// Timer/Counter0 Output Compare A Match Interrupt Enabled
	TIMSK0 |= (1 << OCIE0B);		// Timer/Counter0 Output Compare B Match Interrupt Enabled
	
}

void timer2_init(){
	
	/*** TCCR2A: TC2 Control Register A ***/
	// Operate at Normal Timer Mode...
	/*** TCCR2B: TC2 Control Register B ***/
	/** Prescaler Selection **/
	TCCR2B |= (1 << CS22);			// Set prescaler to 64
	/*** Compare Match Register ***/
	OCR2A = 0;
	OCR2B = 64;
	/*** TIMSK2: TC2 Interrupt Mask Register ***/
	TIMSK2 |= (1 << OCIE2A);		// Timer/Counter0 Output Compare A Match Interrupt Enabled
	TIMSK2 |= (1 << OCIE2B);		// Timer/Counter0 Output Compare B Match Interrupt Enabled
	
}
// NOTE
// PWM Period should be 8.2ms approximately at 256 prescaler.
// Fpwm = F_CPU / (N*256) where N is prescaler value.