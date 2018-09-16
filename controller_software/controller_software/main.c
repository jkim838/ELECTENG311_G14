/*
 * main.c
 *
 * Created: 14/09/2018 8:12:55 AM
 * Author : Oliver K. jkim838 846548800
 * Revision 1.1.6
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
#define ENABLE_DEBUGGING_PARAMETER
#define XPLAINED_MINI_LED_STROBE
#define TIMER_DEBUG_MODE
#define ADC_DEBUG_MODE_MASTER
#ifdef ADC_DEBUG_MODE_MASTER
	#define BAUD_RATE 9600
	#define debug_UBRR F_CPU / 16 / BAUD_RATE -1
	#define debug_PWM_LIVE_TIME 0.006
	#define debug_PWM_PERIOD 0.02 // Period of the PWM. Not the period of the Motor
	#define debug_COIL_CURRENT 0.5 // Be mindful that this is a voltage level across the Current Shunt Resistor
#endif

/*** Debugger Header ***/
#ifdef MAIN_DEBUG_MODE
	#include <util/delay.h>
	#include "debug_usart.h"		// Contains functions to debug ADC with USART Transmission
	#include <stdbool.h>
#endif

/* ISR Variable Definitions */
volatile uint8_t raw_hall_voltages[2];
volatile uint8_t data_collected = 0;
volatile uint8_t raw_coil_voltage;
volatile uint8_t raw_coil_current;
#ifdef MAIN_DEBUG_MODE
	int usart_putchar_printf(char var, FILE *stream);
	static FILE mystdout = FDEV_SETUP_STREAM(usart_putchar_printf, NULL, _FDEV_SETUP_WRITE);
#endif
#ifdef ADC_DEBUG_MODE_MASTER
	uint16_t raw_ADC_output;
	uint8_t debug_ADC_channel;
#endif

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
	timer0_init();	// Set up Timer 0 and Pulse Width Modulation.
	timer2_init();	// Set up Timer 2 and Pulse Width Modulation
	adc_init();		// Set up ADC
	#ifdef ADC_DEBUG_MODE_MASTER
		stdout = &mystdout;
		// enable USART for transmitting digital conversion result to PuTTy...
		debug_usart_init(debug_UBRR);
	#endif
	
	sei();
	
    /* Main Loop */
    while (1) {
		
		#ifdef ENABLE_DEBUGGING_PARAMETER
			uint8_t parameter1;
			uint8_t parameter2;
		#endif
		
		#ifdef MAIN_DEBUG_MODE
		//debug mode... ignore normal operational cycle
			#ifdef ADC_DEBUG_MODE_MASTER
				// try analog to digital conversion on the ADC, and display its output to the PuTTy.
				cli();
				printf("Current ADC Channel: %d\n", debug_ADC_channel);
				printf("Next ADC Channel: %d\n", ADC_next_channel);
				printf("Raw ADC Output: %d\n", raw_ADC_output);
				double digitized_adc_output = debug_adc_digitize(raw_ADC_output);
				printf("Digitized ADC Output: %fV\n",digitized_adc_output);
				double expected_power = calculate_power(digitized_adc_output, debug_COIL_CURRENT, debug_PWM_LIVE_TIME, debug_PWM_PERIOD);
				printf("Expected Power Consumption: %fW\n\n", expected_power);
				sei();
			#endif
		
		#else
		if(data_collected == 4){
			cli();
			data_collected = 0; //reset data collection count
			double stroke_length = calculate_length(raw_hall_voltages);
			double stroke_period = calculate_period(raw_hall_voltages);
			double stroke_frequency = 1/stroke_period;
			double power = calculate_power(raw_coil_voltage, raw_coil_current, parameter1, stroke_period);
			double flow_rate = calculate_flow_rate(parameter1, parameter2);
			sei();
		}
		#endif
		
		#ifdef XPLAINED_MINI_LED_STROBE
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
	#else
	// Normal Operation Mode...
	OCR0A = new_PWM_frequency;			// this parameter needs a function to determine its values.
	#endif
	
}

ISR(TIMER2_OVF_vect){
	
	#ifdef TIMER_DEBUG_MODE
	// Debugger Mode...
	#else
	// Normal Operation Mode
	OCR2A = new_PWM_frequency;
	#endif
	
}

/*** Analog to Digital Conversion Complete Interrupt ***/
ISR(ADC_vect){
	
	#ifdef ADC_DEBUG_MODE_MASTER
	// Debugger Mode...
	// ADC channel switch mode is disabled for the purpose of debugging the ADC.
	// When conversion for right hall effect sensor is complete
		if(ADC_next_channel == 0){
			debug_ADC_channel = ADC_next_channel;
			ADC_next_channel = 2;// Next conversion is coil voltage shunt.
			ADMUX &= 0xf0;			// Reset to Channel 0
			ADMUX |= 0x02;			// Set to Channel 2. (coil voltage shunt)
		}
		else if(ADC_next_channel == 2){
			debug_ADC_channel = ADC_next_channel;
			ADC_next_channel = 3;// Next conversion is coil current shunt
			ADMUX &= 0xf0;			// Reset to Channel 0
			ADMUX |= 0x03;			// Set to Channel 3. (coil current shunt)
		}
		else if(ADC_next_channel == 3){
			debug_ADC_channel = ADC_next_channel;
			ADC_next_channel = 5;// Next conversion is back to right hall effect sensor.
			ADMUX &= 0xf0;			// Reset to Channel 0.
			ADMUX |= 0x05;			// Set to Channel 5. (right hall effect sensor)
		}
		else if(ADC_next_channel == 5){
			raw_ADC_output = ADC;
			debug_ADC_channel = ADC_next_channel;
			ADC_next_channel = 0;// Next conversion is left hall effect sensor.
			ADMUX &= 0xf0;			// Reset to Channel 0. (left hall effect sensor)
		}
		ADCSRA |= (1 << ADSC);
	#else
		// Normal Operation Mode...
		// When conversion for left hall effect sensor is complete...
		if(ADC_next_channel == 0){
			raw_hall_voltages[1] = ADC;
			data_collected++;		// Second data (left hall sensor) collected.
			ADC_next_channel = 2;	// Next conversion is coil voltage shunt.
			ADMUX &= 0xf0;			// Reset to Channel 0
			ADMUX |= 0x02;			// Set to Channel 2. (coil voltage shunt)
		}
		// When conversion for coil voltage shunt is complete...
		else if(ADC_next_channel == 2){
			raw_coil_voltage = ADC;
			data_collected++;		// Third data (shunt voltage) collected.
			ADC_next_channel = 3;	// Next conversion is coil current shunt
			ADMUX &= 0xf0;			// Reset to Channel 0
			ADMUX |= 0x03;			// Set to Channel 3. (coil current shunt)
		}
		// When conversion for coil current shunt is complete...
		else if(ADC_next_channel == 3){
			raw_coil_current = ADC;
			data_collected++;		// Fourth data (shunt current) collected.
			ADC_next_channel = 5;	// Next conversion is back to right hall effect sensor.
			ADMUX &= 0xf0;			// Reset to Channel 0.
			ADMUX |= 0x05;			// Set to Channel 5. (right hall effect sensor)
		}
		// When conversion for right hall effect sensor is complete
		else if(ADC_next_channel == 5){
			raw_hall_voltages[0] = ADC;
			data_collected++;		// First data (right hall effect) collected
			ADC_next_channel = 0;	// Next conversion is left hall effect sensor.
			ADMUX &= 0xf0;			// Reset to Channel 0. (left hall effect sensor)
		}
		ADCSRA |= (1 << ADSC);
	#endif
	
}
// NOTE: PWM Output Gate for Timer 0 is PD6
// NOTE: PWM Output Gate for Timer 2 is PB3