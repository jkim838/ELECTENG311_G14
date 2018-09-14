/*
 * main.c
 *
 * Created: 14/09/2018 8:12:55 AM
 * Author : Oliver K. jkim838 846548800
 * Revision 1.1.4
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
#include "adc_setup.h"			// Contains functions used to configure ADC
#include "calculations.h"		// Contains functions to calculate parameters

/*** Custom Macro Definitions ***/
#define F_CPU 16000000UL
#define MAIN_DEBUG_MODE
#define TIMER_DEBUG_MODE
#define ADC_DEBUG_MODE

/*** Delay Header ***/
#include <util/delay.h>

/* ISR Variable Definitions */
volatile double hall_voltages[2];
volatile uint8_t data_collected = 0;
volatile double coil_voltage;
volatile double coil_current;

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
		
	/* ATMEGA328P Module Initialization */
	// Remove double slashes to activate...
	timer_init();	// Set up Timer and Pulse Width Modulation.
	
	sei();
	
    /* Main Loop */
    while (1) {
		
		#ifdef MAIN_DEBUG_MODE
			uint8_t parameter1;
			uint8_t parameter2;
		#endif
		
		// Check if hall effect sensor voltage array is complete
		// (i.e. left / right hall effect sensor voltages are available)
		if(data_collected == 4){
			cli();
			data_collected = 0; //reset data collection count
			double stroke_length = calculate_length(hall_voltages);
			double stroke_period = calculate_period(hall_voltages);
			double stroke_frequency = 1/stroke_period;
			double power = calculate_power(coil_voltage, coil_current);
			double flow_rate = calculate_flow_rate(parameter1, parameter2);
			sei();
		}
		
		#ifdef MAIN_DEBUG_MODE
			PORTB ^= (1 << PB5);
			_delay_ms(100);
		#endif
		

    }
	
	cli();
	
	return 0;
}

/*** Interrupt Service Routine Definitions ***/
ISR(TIMER0_OVF_vect){
	
	#ifdef TIMER_DEBUG_MODE
	// Debugger Mode...
	PORTB ^= (1 << PB4);
	
	#else
	// Normal Operation Mode...
	OCR0A = new_PWM_frequency;
	
	#endif
}

/*** Analog to Digital Conversion Complete Interrupt ***/
ISR(ADC_vect){
	
	#ifdef ADC_DEBUG_MODE
	// Debugger Mode...
	// When conversion for left hall effect sensor is complete...
	if(ADC_current_channel == 0){
		
	}
	// When conversion for coil voltage shunt is complete...
	else if(ADC_current_channel == 2){
		
	}
	// When conversion for coil current shunt is complete...
	else if(ADC_current_channel == 3){
		
	}
	// When conversion for right hall effect sensor is complete
	else if(ADC_current_channel == 5){
		
	}
	
	#else
	// Normal Operation Mode...
	// When conversion for left hall effect sensor is complete...
	if(ADC_current_channel == 0){
		hall_voltages[1] = ADC;
		data_collected++;		// Second data (left hall sensor) collected.
		ADC_current_channel = 2;	// Next conversion is coil voltage shunt.
		ADMUX &= 0xf0;			// Reset to Channel 0
		ADMUX |= 0x02;			// Set to Channel 2. (coil voltage shunt)
	}
	// When conversion for coil voltage shunt is complete...
	else if(ADC_current_channel == 2){
		coil_voltage = ADC;
		data_collected++;		// Third data (shunt voltage) collected.
		ADC_current_channel = 3;	// Next conversion is coil current shunt
		ADMUX &= 0xf0;			// Reset to Channel 0
		ADMUX |= 0x03;			// Set to Channel 3. (coil current shunt)
	}
	// When conversion for coil current shunt is complete...
	else if(ADC_current_channel == 3){
		coil_current = ADC;
		data_collected++;		// Fourth data (shunt current) collected.
		ADC_current_channel = 0;	// Next conversion is back to right hall effect sensor.
		ADMUX &= 0xf0;			// Reset to Channel 0.
		ADMUX |= 0x05;			// Set to Channel 5. (right hall effect sensor)
	}
	// When conversion for right hall effect sensor is complete
	else if(ADC_current_channel == 5){
		hall_voltages[0] = ADC;
		data_collected++;		// First data (right hall effect) collected
		ADC_current_channel = 2;	// Next conversion is left hall effect sensor.
		ADMUX &= 0xf0;			// Reset to Channel 0. (left hall effect sensor)
	}
	
	#endif
	
}
// NOTE: PWM Output Gate is PD6