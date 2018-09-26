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
	
	TCCR0A |= (1 << WGM02);
	TCCR0B |= (1 << CS01);
	TCCR0B |= (1 << CS00);
	OCR0A = 0;
	OCR0B = 64;
	TIMSK0 |= (1 << OCIE0A);
	TIMSK0 |= (1 << OCIE0B);
	
}

void timer2_init(){
	
    TCCR2B |= (1 << WGM22);
	TCCR2B |= (1 << CS22);
	OCR2A = 0;
	OCR2B = 64;
	TIMSK2 |= (1 << OCIE2A);
	TIMSK2 |= (1 << OCIE2B);
	
}
// NOTE
// PWM Period should be 8.2ms approximately at 256 prescaler.
// Fpwm = F_CPU / (N*256) where N is prescaler value.