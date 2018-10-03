/*
 * main.c
 *
 * Created: 14/09/2018 8:12:55 AM
 * Author : Oliver K. jkim838 846548800
 * Revision 1.3.0
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
#include <stdbool.h>
#include <string.h>
/*** Custom Header Files ***/
#include "timer_setup.h"		// Contains functions used to configure timer
#include "adc_setup.h"			// Contains functions used to configure ADC
#include "calculations.h"		// Contains functions to calculate parameters
#include "Macro_Definitions.h"  // Contains all necessary Macro definitions
#include "mjson.h"				// Contains functions for JSON communications
/*** Debugger Header ***/
#ifdef TRANSMIT_DEBUG_MODE
	#include "debug_usart.h"		// Contains functions to transmit variable values with USART Transmission
#else
	#include "Comm_Setup.h"			// Contains functions to communicate between Master/Slave System.	
#endif


#include <util/delay.h>

/*** ISR Variable Definitions ***/

/** Analog-Digital Converter ISR **/
volatile uint16_t raw_coil_voltage;
volatile uint16_t raw_coil_current;
volatile uint16_t raw_coil_voltages[24];
volatile uint16_t raw_coil_currents[24];
volatile uint8_t samples_counter = 0;
volatile uint8_t raw_coil_voltage_index = 0;
volatile uint8_t raw_coil_current_index = 0;
volatile uint16_t raw_maximum_voltage;
volatile uint16_t raw_minimum_voltage;
volatile uint8_t debug_ADC_channel;

/** Timer/Pulse Modulation ISR **/
volatile uint8_t MATCH_COUNTER_T0 = 0;
volatile uint8_t MATCH_COUNTER_T2 = 0;
volatile uint8_t PULSE_0_START_TIME = 0;
#ifdef XPLAINED_MINI_MODE
	volatile uint8_t PULSE_0_REACTIVATE_TIME = 134;  //in milliseconds, state the next pulse0 HIGH point
	volatile uint8_t PULSE_2_START_TIME = 67;		//(do integer calculation) pulse 0 reactivate time / 2 + 1
	volatile uint8_t PULSE_KILL_TIME = 34;			// duty cycle based on reactivate time
#else
	volatile uint8_t PULSE_0_REACTIVATE_TIME = 50;  //in milliseconds, state the next pulse0 HIGH point
	volatile uint8_t PULSE_2_START_TIME = 25;		//(do integer calculation) pulse 0 reactivate time / 2 + 1
	volatile uint8_t PULSE_KILL_TIME = 13;			// duty cycle based on reactivate time
#endif


/** Master Debug Routine ISR **/
#ifdef TRANSMIT_DEBUG_MODE
#else
	volatile uint8_t usart_RX_index = 0;
	volatile unsigned char usart_RX[JSON_FIXED_BUFFER_SIZE];			// The maximum assumed size of Rx buffer.
	volatile unsigned char usart_RX_Complete[JSON_FIXED_BUFFER_SIZE];	// A copy of a completed JSON Transmission to be used by the main loop
	volatile bool RX_sequence_complete = false;
#endif

/*** Global Variable Definitions ***/
#ifdef TRANSMIT_DEBUG_MODE
	int usart_putchar_printf(char var, FILE *stream);
	static FILE mystdout = FDEV_SETUP_STREAM(usart_putchar_printf, NULL, _FDEV_SETUP_WRITE);
#endif

#ifdef ADC_DEBUG_MODE
	uint16_t raw_ADC_output_PC0;
	uint16_t raw_ADC_output_PC5;
#endif

int main(void){
	
	/*** Run Once Code ***/
	/* PIN I/O Setup */
	// Set all pins to output. Change mode to input when necessary
	DDRB = 0xff;
	DDRC = 0xff;
	DDRD = 0xff;
	
	// Set ADC Input Gates...
	DDRC &= ~(1 << PC0); // ADC Channel 0... Voltage Shunt Channel
	DDRC &= ~(1 << PC5); // ADC Channel 5... Current Shunt Channel
	#ifdef TRANSMIT_DEBUG_MODE
		DDRB &= ~(1 << PB7); // Debug Button
	#endif 
		
	/* ATMEGA328P Module Initialization */
	// Remove double slashes to activate...
	timer2_init();	// Set up Timer 0 for Pulse Modulation
	adc_init();		// Set up ADC
	#ifdef TRANSMIT_DEBUG_MODE
		stdout = &mystdout;
		// enable USART for transmitting digital conversion result to PuTTy...
		debug_usart_init(debug_UBRR);
	#else
		usart_init(UBRR_VALUE);
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
				cli();
				// try analog to digital conversion on the ADC, and display its output to the PuTTy.
				double digitized_adc_output_PC0 = debug_adc_digitize(raw_ADC_output_PC0);
				double digitized_adc_output_PC5 = debug_adc_digitize(raw_ADC_output_PC5);
				double maximum_voltage = debug_adc_digitize(raw_maximum_voltage);
				double minimum_voltage = debug_adc_digitize(raw_minimum_voltage);
				double expected_power = calculate_power(digitized_adc_output_PC0, debug_COIL_CURRENT, debug_PWM_LIVE_TIME, debug_PWM_PERIOD);
				#ifdef ENABLE_PRINTF
					printf("Current Channel: %d\n", debug_ADC_channel);
					printf("Next Channel: %d\n", ADC_next_channel);
					printf("ADC0 Output (PC0): %f\nADC5 Output (PC5): %f\n", digitized_adc_output_PC0, digitized_adc_output_PC5);
					printf("Maximum Voltage: %f\nMinimum Voltage: %f\n", maximum_voltage,minimum_voltage);
					#ifdef CALCULATION_DEBUG_MODE
						printf("Estimated Power Consumption: %f\n", expected_power);
					#endif
				#endif
				if(samples_counter == 24){
					samples_counter = 0;
					raw_coil_voltage_index = 0;
					raw_coil_current_index = 0;
				}
				#ifdef ENABLE_PRINTF
					printf("\n");
				#endif
				sei();
			#endif
		
		#else
		// normal operation mode
			// constantly calculate the parameter values
			double digitized_adc_output_PC0 = debug_adc_digitize(raw_ADC_output_PC0);
			double digitized_adc_output_PC5 = debug_adc_digitize(raw_ADC_output_PC5);
			double maximum_voltage = debug_adc_digitize(raw_maximum_voltage);
			double minimum_voltage = debug_adc_digitize(raw_minimum_voltage);
			double expected_power = calculate_power(digitized_adc_output_PC0, debug_COIL_CURRENT, debug_PWM_LIVE_TIME, debug_PWM_PERIOD);
			
			// only read from the buffer when RX is complete...
			if(RX_sequence_complete == true){
				cli();
				// read from the buffer...
				
				uint8_t digit_req[3];
				usart_obtain_req(digit_req, usart_RX);
				uint8_t numerical_req = numerify_req(digit_req);
				
				// prepare report... (calculation etc...)
				// transmit report...
				RX_sequence_complete = false;
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
#ifdef TRANSMIT_DEBUG_MODE
	// Do not setup communication program when in debugging mode.
#else

	/*** USART Receive Complete Description ***/
	/* USART on ATMEGA328P microcontroller is currently set to transmit and receive 8-bit ASCII characters across Master (PC or equivalent system) and Slave 
	(Linear Motor Controller) in JSON structure at a baud-rate of 9600.
	
	The input length to slave system is consistent in length of minimum 25 characters (in ASCII) and maximum 27 characters. When slave successfully receive 8-bit
	transmission from the master, USART Receive Complete flag is set.
	
	a. The received data (an ASCII character) is stored in an unsigned character buffer (usartRX).
	b. The index counter for the buffer is incremented for the upcoming data.
	c. On the 28th count (i.e. one sequence of master-slave communication is complete), reset the index position to overwrite the first character of the buffer.
	 */
	ISR(USART_RX_vect){
		// Reception is complete... Need to find a way to extract information.
		if(usart_RX_index != JSON_FIXED_BUFFER_SIZE){				// Starting from index of zero, the buffer will be filled with 
			usart_RX[usart_RX_index] = UDR0;						// transmission upto 37th index.
		}
		else if (usart_RX_index == JSON_FIXED_BUFFER_SIZE){
			// If the index counter reaches 38, which is larger than the maximum length of the fixed JSON RX...
			usart_RX_index = 0;										 // Reset the Index back to zero (hence allow new sequence of RX)...
			usart_RX[usart_RX_index] = UDR0;						 // Record the first bit of the RX to the buffer...
			RX_sequence_complete = true;							 // Flag to main loop that one cycle of 
		}
		usart_RX_index++;
	}
	
#endif

#ifdef ENABLE_PRINTF
	// Pulse Output is disabled due to unexpected behavior caused by long delays between global interrupt disabled and enabled.
	
#else

	/*** Pulse Modulation Unit Description ***/
	/*Two 8-bit TIMER/COUNTER on ATMEGA328P microcontroller is currently set to produce Compare Match Flag each time TCNTn register
	value is equal to Output Compare Register A (OCR0A, OCR2A) and Output Compare Register B (OCR0B, OCR2B), at approximately 1 millisecond
	(1 ms) on 16 MHz main clock speed.
	
	The duration between each Compare Match Flag A and Compare Match Flag B is adjustable by changing the value of Output Compare Register B
	of the timers. For example, On a slower system, operating at lower clock frequency (e.g. PCB Mounted ATMEGA 328P @ 8 MHz), the same 1 ms 
	period can be produced by reducing the OCR0B and OCR2B values by half.
	
	a. At every compare Match Flag A, the designated output pin will produce 5V (HIGH).
	b. At every Compare Match Flag B, the match counter variable (MATCH_COUNTER_T0 for TIMER0, MATCH_COUNTER_T2 for TIMER2) is incremented, indicating 
	   how many milliseconds has passed since the pin was activated.
	c. The live time of the output is controlled by comparing the counter variable to a desired output duration: for example, six match counter is 
	   required to produce 6 milliseconds of live-time. 
	d. The total period (Live-time + Dead-time) between each TIMER output is controlled by comparing the counter variable to a desired period: for
	   example, //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////PLEASE UPDATE THIS COMMENT
	   
	/***WARNING***/
	/* The following conditions are critical to correct operation of the system. TIMER2 Output can be offsetted by desired amount to produce asynchronous output to TIMER0
	a. To ensure correct pulse output, both TIMER0 and TIMER2 operation must be synchronized upon initialization.
	b. When macro ENABLE_PRINTF is defined, Pulse Modulation will behave unexpectedly due to long delay between global interrupt disable and global 
	   interrupt enable caused by printf function. To ensure correct functionality of the Pulse Modulation, Macro ENABLE_PRINTF must first be undefined.*/
	
	ISR(TIMER2_COMPA_vect){
		if(MATCH_COUNTER_T0 == PULSE_0_START_TIME){
			PORTB &= ~(1 << PB3);
			PORTD |= (1 << PD6);						// Activate Output PD6
		}
		else if(MATCH_COUNTER_T0 == PULSE_2_START_TIME){
			// debug... make PB3 high after 1/2 of period we desire
			PORTD &= ~(1 << PD6);
			PORTB |= (1 << PB3);
		}
		else if(MATCH_COUNTER_T0 == PULSE_0_REACTIVATE_TIME){
			PORTB &= ~(1 << PB3);
			PORTD |=  (1 << PD6);
			MATCH_COUNTER_T0 = PULSE_0_START_TIME;
		}
	}

	ISR(TIMER2_COMPB_vect){
		if(MATCH_COUNTER_T0 == PULSE_KILL_TIME){
			PORTD &= ~(1 << PD6);						// Deactivate Output PD6
		}
		if(MATCH_COUNTER_T0 == PULSE_KILL_TIME + PULSE_2_START_TIME){
			PORTB &= ~(1 << PB3);
		}
		MATCH_COUNTER_T0++;
	}
	
#endif

/*** Analog to Digital Conversion Complete Interrupt ***/
ISR(ADC_vect){
	
	/*** Analog-Ditical Converter Description ***/
	/* The Analog-Digital Converter on ATMEGA328P is currently set to take inputs from two channels: Channel 0 (located PD5) and Channel 5 (located PD0).
	Channel 0 is assigned for coil voltage sensing.
	Channel 5 is assigned for coil current sensing. 
	
	a. Upon activation, analog-digital converter is initially set to receive input from Channel 0 (coil voltage). 
	b. At every completion of conversion, an interrupt flag is set. The output from ADC Register (ADC) is assigned to appropriate variables (raw_coil_voltage OR raw_coil_current).
	c. After ADC register values are assigned to a variable, channel switching occurs. Channels alternate between 0 to 5, and vice versa.
	d. A new conversion starts.
	e. The acquired ADC Register values are stored in the variable until next cycle of conversion is complete. It is passed down to main loop for fabrication.*/
	
	/*** WARNING ***/
	/* The following conditions are critical to correct operation of the system.
	a. As of 27.09.18. the feature to sample 24 points across the input waveform is not yet tested. */
	#ifdef ADC_DEBUG_MODE
	// Debugger Mode...
		
		if (samples_counter == 0 && ADC_next_channel == ADC_COIL_VOLTAGE_CHANNEL){
			raw_maximum_voltage = ADC;						// Note: Only input in Channel 0 will be considered
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
			raw_ADC_output_PC0 = ADC;
			debug_ADC_channel = ADC_next_channel;
			ADC_next_channel = ADC_COIL_CURRENT_CHANNEL;	// Next conversion is coil current shunt
			ADMUX &= 0xf0;									// Reset to Channel 0
			ADMUX |= 0x05;									// Set to Channel 5. (coil current shunt)
		}
		else if(ADC_next_channel == ADC_COIL_CURRENT_CHANNEL){
			if(raw_coil_current_index < 24){
				raw_coil_currents[raw_coil_current_index] = ADC;
			}
			raw_coil_current_index++;
			raw_ADC_output_PC5 = ADC;
			debug_ADC_channel = ADC_next_channel;
			ADC_next_channel = ADC_COIL_VOLTAGE_CHANNEL;	// Next conversion is back to coil voltage shunt
			ADMUX &= 0xf0;									// Reset to Channel 0.
			ADMUX |= 0x00;									// Set to Channel 0. (coil voltage shunt)
		}
		samples_counter++;
		ADCSRA |= (1 << ADSC);								// Start a new conversion...
		
	#else
		
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
			raw_ADC_output_PC0 = ADC;
			ADC_next_channel = ADC_COIL_CURRENT_CHANNEL;	// Next conversion is coil voltage shunt
			ADMUX &= 0xf0;									// Reset to Channel 0
			ADMUX |= 0x00;									// Set to Channel 0. (coil voltage shunt)
		}
		else if(ADC_next_channel == ADC_COIL_CURRENT_CHANNEL){
			if(raw_coil_current_index < 24){
				raw_coil_currents[raw_coil_current_index] = ADC;
			}
			raw_coil_current_index++;
			raw_ADC_output_PC5 = ADC;
			ADC_next_channel = ADC_COIL_VOLTAGE_CHANNEL;	// Next conversion is back to coil current shunt
			ADMUX &= 0xf0;									// Reset to Channel 0.
			ADMUX |= 0x05;									// Set to Channel 5. (coil current shunt)
		}
		samples_counter++;	
		ADCSRA |= (1 << ADSC);								// Start a new conversion...
	#endif
	
}
// NOTE: PWM Output Gate for Timer 0 is PD6
// NOTE: PWM Output Gate for Timer 2 is PB3