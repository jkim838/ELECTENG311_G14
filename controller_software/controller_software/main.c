/*
 * main.c
 *
 * Created: 14/09/2018 8:12:55 AM
 * Author : Oliver K. jkim838 846548800
 * Revision 1.1.3
 *
 * Description:
 * Primary program to control the operation condition of the linear compressor motor unit.
 *
 */ 

/*** Header Files ***/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdio.h>

/*** Custom Header Files ***/
// Remove double slash to activate
#include "timer_setup.h"		// Contains functions used to configure timer

/*** Custom Macro Definitions ***/
#define F_CPU 16000000UL
#define DEBUG_MODE
/*** Delay Header ***/
#include <util/delay.h>

/*** Interrupt Service Routine Definitions ***/
ISR(TIMER0_OVF_vect){
	
	#ifdef DEBUG_MODE
		PORTB ^= (1 << PB4);
	
	#else
		OCR0A = new_PWM_frequency;
		
	#endif
}
/*** Analog to Digital Conversion Complete Interrupt ***/


int main(void){
	
	/*** Run Once Code ***/
	/* PIN I/O Setup */
	// Set all pins to output. Change mode to input when necessary
	DDRB = 0xff;
	DDRC = 0xff;
	DDRD = 0xff;
	
	// Set ADC Input Gates...
	DDRC &= ~(1 << PC0); // ADC Channel 0... left hall effect sensor
	DDRC &= ~(1 << PC2); // ADC Channel 2... Coil Voltage Shunt
	DDRC &= ~(1 << PC3); // ADC Channel 3... Coil Current Shunt
	DDRC &= ~(1 << PC5); // ADC Channel 5... Right hall effect sensor
	
	/* Global Variable Definitions */
	// Remove double slashes to activate...
	
	/* ATMEGA328P Module Initialization */
	// Remove double slashes to activate...
	timer_init();	// Set up Timer and Pulse Width Modulation.
	
	sei();
	
    /* Main Loop */
    while (1) {
		
		#ifdef DEBUG_MODE
			PORTB ^= (1 << PB5);
			_delay_ms(100);
		#endif
    }
	
	cli();
	
	return 0;
}

// NOTE: PWM Output Gate is PD6