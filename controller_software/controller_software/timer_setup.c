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

void timer2_init(){
	
	/*** TCCR2A: TC2 Control Register A ***/
	// Operate at Normal Timer Mode...
	/*** TCCR2B: TC2 Control Register B ***/
	/** Prescaler Selection **/
	#ifdef XPLAINED_MINI_MODE
	// The program is being tested on a Xplained Mini Board, and it is running at 16MHz.
		TCCR2B |=  (1 << CS22);			// Set prescaler to 64
		TCCR2B &= ~(1 << CS21);			// @F_CPU 16 MHz... F(Timer) = 250KHz
		TCCR2B &= ~(1 << CS20);			
	#else
	// The program is running at 8MHz.
		TCCR2B &= ~(1 << CS22);			// Set prescaler to 32
		TCCR2B |=  (1 << CS21);			// @F_CPU 8 MHz... F(Timer) = 250KHz
		TCCR2B |=  (1 << CS20);		    
	#endif
	/*** TIMSK0: TC0 Interrupt Mask Register ***/
	TIMSK2 |= (1 << OCIE2A);		// Timer/Counter2 Output Compare A Match Interrupt Enabled
	TIMSK2 |= (1 << OCIE2B);		// Timer/Counter2 Output Compare B Match Interrupt Enabled
	
}

// NOTE