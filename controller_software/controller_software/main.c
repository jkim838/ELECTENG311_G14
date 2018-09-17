/*
 * main.c
 *
 * Created: 14/09/2018 8:12:55 AM
 * Author : Oliver K. jkim838 846548800
 * Revision 1.2.4
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
#include "timer_setup.h"		// Contains functions used to configure timer
#include "adc_setup.h"			// Contains functions used to configure ADC
#include "calculations.h"		// Contains functions to calculate parameters
#include "Macro_Definitions.h"  // Contains all necessary Macro definitions

/*** Debugger Header ***/
#ifdef TRANSMIT_DEBUG_MODE
	#include <util/delay.h>
	#include "debug_usart.h"		// Contains functions to transmit variable values with USART Transmission
	#include <stdbool.h>
#endif

/*** ISR Variable Definitions ***/
#ifdef DISABLE_HALL_EFFECT_SENSORS
	volatile uint16_t raw_hall_voltages[2];
#endif
volatile uint8_t data_collected = 0;
volatile uint16_t raw_coil_voltage;
volatile uint16_t raw_coil_current;
volatile uint16_t raw_coil_voltages[24];
volatile uint16_t raw_coil_currents[24];
volatile uint8_t samples_counter = 0;
volatile uint8_t raw_coil_voltage_index = 0;
volatile uint8_t raw_coil_current_index = 0;
volatile uint16_t raw_maximum_voltage;
volatile uint16_t raw_minimum_voltage;  

/*** Global Variable Definitions ***/
#ifdef TRANSMIT_DEBUG_MODE
	int usart_putchar_printf(char var, FILE *stream);
	static FILE mystdout = FDEV_SETUP_STREAM(usart_putchar_printf, NULL, _FDEV_SETUP_WRITE);
#endif

#ifdef ADC_DEBUG_MODE
	#ifdef DISABLE_HALL_EFFECT_SENSORS
		uint16_t raw_ADC_output_PC2;
		uint16_t raw_ADC_output_PC3;
	#else
		uint16_t raw_ADC_output_PC0;
		uint16_t raw_ADC_output_PC2;
		uint16_t raw_ADC_output_PC3;
		uint16_t raw_ADC_output_PC5;
	#endif
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
	#ifdef TRANSMIT_DEBUG_MODE
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
		
		#ifdef TRANSMIT_DEBUG_MODE
		//debug mode... ignore normal operational cycle
			#ifdef ADC_DEBUG_MODE
				// try analog to digital conversion on the ADC, and display its output to the PuTTy.
				printf("Current ADC Channel: %d\n", debug_ADC_channel);
				printf("Next ADC Channel: %d\n", ADC_next_channel);
				#ifdef DISABLE_HALL_EFFECT_SENSORS
					printf("Channel 2 Raw ADC Output: %d\n", raw_ADC_output_PC2);
					printf("Channel 3 Raw ADC Output: %d\n", raw_ADC_output_PC3);
					double digitized_adc_output_PC2 = debug_adc_digitize(raw_ADC_output_PC2);
					double digitized_adc_output_PC3 = debug_adc_digitize(raw_ADC_output_PC3);
					printf("Digitized ADC Output: %fV\n",digitized_adc_output_PC2);
					printf("Digitized ADC Output: %fV\n",digitized_adc_output_PC3);
					#ifdef CALCULATION_DEBUG_MODE
						double expected_power = calculate_power(digitized_adc_output_PC2, debug_COIL_CURRENT, debug_PWM_LIVE_TIME, debug_PWM_PERIOD);
						printf("Expected Power Consumption: %fW\n", expected_power);
					#endif
					double maximum_voltage = debug_adc_digitize(raw_maximum_voltage);
					double minimum_voltage = debug_adc_digitize(raw_minimum_voltage);
					printf("Maximum Voltage Detected: %f\n", maximum_voltage);
					printf("Minimum Voltage Detected: %f\n", minimum_voltage);
					if(samples_counter == 24){
						samples_counter = 0;
						raw_coil_voltage_index = 0;
						raw_coil_current_index = 0;
					}
				#else
					printf("Channel 0 Raw ADC Output: %d\n", raw_ADC_output_PC0);
					printf("Channel 2 Raw ADC Output: %d\n", raw_ADC_output_PC2);
					printf("Channel 3 Raw ADC Output: %d\n", raw_ADC_output_PC3);
					printf("Channel 5 Raw ADC Output: %d\n", raw_ADC_output_PC5);
					double digitized_adc_output_PC0 = debug_adc_digitize(raw_ADC_output_PC0);
					double digitized_adc_output_PC2 = debug_adc_digitize(raw_ADC_output_PC2);
					double digitized_adc_output_PC3 = debug_adc_digitize(raw_ADC_output_PC3);
					double digitized_adc_output_PC5 = debug_adc_digitize(raw_ADC_output_PC5);
					printf("Digitized ADC Output: %fV\n",digitized_adc_output_PC0);
					printf("Digitized ADC Output: %fV\n",digitized_adc_output_PC2);
					printf("Digitized ADC Output: %fV\n",digitized_adc_output_PC3);
					printf("Digitized ADC Output: %fV\n",digitized_adc_output_PC5);
					#ifdef CALCULATION_DEBUG_MODE
						double expected_power = calculate_power(digitized_adc_output_PC5, debug_COIL_CURRENT, debug_PWM_LIVE_TIME, debug_PWM_PERIOD);
						printf("Expected Power Consumption: %fW\n", expected_power);
					#endif
				#endif
				printf("\n");
				_delay_ms(100);
			#endif
		
		#else
		// normal operation mode
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
	
	#ifdef ADC_DEBUG_MODE
	// Debugger Mode...
		#ifdef DISABLE_HALL_EFFECT_SENSORS
			if (samples_counter == 0 && ADC_next_channel == ADC_COIL_VOLTAGE_CHANNEL){
				raw_maximum_voltage = ADC;	// Note: Only input in Channel 2 will be considered
				raw_minimum_voltage = ADC;
			}
			if (ADC_next_channel == ADC_COIL_VOLTAGE_CHANNEL){
				if(raw_coil_voltage_index < 24){
					raw_coil_voltages[raw_coil_voltage_index] = ADC;
				}
				if(ADC > raw_maximum_voltage){
					raw_maximum_voltage = ADC;
				}
				if(ADC < raw_minimum_voltage){
					raw_minimum_voltage = ADC;
				}
				raw_coil_voltage_index++;
				raw_ADC_output_PC2 = ADC;
				debug_ADC_channel = ADC_next_channel;
				ADC_next_channel = ADC_COIL_CURRENT_CHANNEL;	// Next conversion is coil current shunt
				ADMUX &= 0xf0;									// Reset to Channel 0
				ADMUX |= 0x03;									// Set to Channel 3. (coil current shunt)
			}
			else if(ADC_next_channel == ADC_COIL_CURRENT_CHANNEL){
				if(raw_coil_current_index < 24){
					raw_coil_currents[raw_coil_current_index] = ADC;
				}
				raw_coil_current_index++;
				raw_ADC_output_PC3 = ADC;
				debug_ADC_channel = ADC_next_channel;
				ADC_next_channel = ADC_COIL_VOLTAGE_CHANNEL;	// Next conversion is back to coil voltage sensor
				ADMUX &= 0xf0;									// Reset to Channel 0.
				ADMUX |= 0x02;									// Set to Channel 5. (right hall effect sensor)
			}
			samples_counter++;
		#else
			// When conversion for right hall effect sensor is complete
			if(ADC_next_channel == ADC_LEFT_HALL_CHANNEL){
				debug_ADC_channel = ADC_next_channel;
				ADC_next_channel = ADC_COIL_VOLTAGE_CHANNEL;	// Next conversion is coil voltage shunt.
				ADMUX &= 0xf0;									// Reset to Channel 0
				ADMUX |= 0x02;									// Set to Channel 2. (coil voltage shunt)
			}
			else if(ADC_next_channel == ADC_COIL_VOLTAGE_CHANNEL){
				debug_ADC_channel = ADC_next_channel;
				ADC_next_channel = ADC_COIL_CURRENT_CHANNEL;	// Next conversion is coil current shunt
				ADMUX &= 0xf0;									// Reset to Channel 0
				ADMUX |= 0x03;									// Set to Channel 3. (coil current shunt)
			}
			else if(ADC_next_channel == ADC_COIL_CURRENT_CHANNEL){
				debug_ADC_channel = ADC_next_channel;
				ADC_next_channel = ADC_RIGHT_HALL_CHANNEL;		// Next conversion is back to right hall effect sensor.
				ADMUX &= 0xf0;									// Reset to Channel 0.
				ADMUX |= 0x05;									// Set to Channel 5. (right hall effect sensor)
			}
			else if(ADC_next_channel == ADC_RIGHT_HALL_CHANNEL){
				raw_ADC_output_PC5 = ADC;
				debug_ADC_channel = ADC_next_channel;
				ADC_next_channel = ADC_LEFT_HALL_CHANNEL;		// Next conversion is left hall effect sensor.
				ADMUX &= 0xf0;									// Reset to Channel 0. (left hall effect sensor)
			}
		#endif
		ADCSRA |= (1 << ADSC);
	#else
		#ifdef DISABLE_HALL_EFFECT_SENSORS
			// Normal Operation Mode (w/o Hall Effect Sensors)
			if (samples_counter == 0){
				raw_maximum_voltage = ADC;
				raw_minimum_voltage = ADC;
			}
			
			if (ADC_next_channel == ADC_COIL_VOLTAGE_CHANNEL){
				if(raw_coil_voltage_index < 24){
					raw_coil_voltages[raw_coil_voltage_index] = ADC;
				}
				if(ADC > raw_maximum_voltage){
					raw_maximum_voltage = ADC;
				}
				if(ADC < raw_minimum_voltage){
					raw_minimum_voltage = ADC;
				}
				raw_coil_voltage_index++;
				raw_ADC_output_PC2 = ADC;
				ADC_next_channel = ADC_COIL_CURRENT_CHANNEL;// Next conversion is coil current shunt
				ADMUX &= 0xf0;			// Reset to Channel 0
				ADMUX |= 0x03;			// Set to Channel 3. (coil current shunt)
			}
			else if(ADC_next_channel == ADC_COIL_CURRENT_CHANNEL){
				if(raw_coil_current_index < 24){
					raw_coil_currents[raw_coil_current_index] = ADC;
				}
				raw_coil_current_index++;
				raw_ADC_output_PC3 = ADC;
				ADC_next_channel = ADC_COIL_VOLTAGE_CHANNEL;// Next conversion is back to right hall effect sensor.
				ADMUX &= 0xf0;			// Reset to Channel 0.
				ADMUX |= 0x02;			// Set to Channel 5. (right hall effect sensor)
			}
			samples_counter++;
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
		#endif
		ADCSRA |= (1 << ADSC);
	#endif
	
}
// NOTE: PWM Output Gate for Timer 0 is PD6
// NOTE: PWM Output Gate for Timer 2 is PB3