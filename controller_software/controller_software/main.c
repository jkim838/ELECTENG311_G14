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


/** Timer/Pulse Modulation ISR **/
volatile uint8_t MATCH_COUNTER_T2 = 0;			// DO NOT CHANGE
volatile uint8_t PULSE_0_START_TIME = 0;		// DO NOT CHANGE
volatile uint8_t PULSE_0_REACTIVATE_TIME = 134; // In milliseconds, state the next PULSE_0 HIGH time.
volatile uint8_t PULSE_2_START_TIME = 67;		// PULSE_0_REACTIVATE_TIME / 2
volatile uint8_t PULSE_KILL_TIME = 34;			// 0.5 * desired duty cycle based on PULSE_0_REACTIVATE_TIME


/** Master Debug Routine ISR **/
#ifdef TRANSMIT_DEBUG_MODE
#else
	volatile uint8_t usart_RX_index = 0;
	volatile unsigned char usart_RX[JSON_FIXED_BUFFER_SIZE];			// The maximum assumed size of Rx buffer.
	volatile bool RX_sequence_complete = false;							// Flow rate request will be read when the buffer is full
#endif

/*** Global Variable Definitions ***/
#ifdef TRANSMIT_DEBUG_MODE
	int usart_putchar_printf(char var, FILE *stream);
	static FILE mystdout = FDEV_SETUP_STREAM(usart_putchar_printf, NULL, _FDEV_SETUP_WRITE);
#endif

#ifdef ADC_DEBUG_MODE
	uint16_t raw_ADC_output_PC0;
	uint16_t raw_ADC_output_PC5;
	volatile uint8_t debug_ADC_channel;
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

				// try analog to digital conversion on the ADC, and display its output to the PuTTy.
				double digitized_adc_output_PC0 = debug_adc_digitize(raw_ADC_output_PC0);
				double digitized_adc_output_PC5 = debug_adc_digitize(raw_ADC_output_PC5);
				double expected_power = calculate_power(digitized_adc_output_PC0, debug_COIL_CURRENT, debug_PWM_LIVE_TIME, debug_PWM_PERIOD);
				#ifdef ENABLE_PRINTF
					cli();
					printf("Current Channel: %d\n", debug_ADC_channel);
					printf("Next Channel: %d\n", ADC_next_channel);
					printf("ADC0 Output (PC0): %f\nADC5 Output (PC5): %f\n", digitized_adc_output_PC0, digitized_adc_output_PC5);
					#ifdef CALCULATION_DEBUG_MODE
						printf("Estimated Power Consumption: %f\n", expected_power);
					#endif
					printf("\n");
					sei();
				#endif
				
			#endif
		
		#else
		// normal operation mode
			// constantly calculate the parameter values
			double digitized_adc_output_PC0 = debug_adc_digitize(raw_ADC_output_PC0);
			double digitized_adc_output_PC5 = debug_adc_digitize(raw_ADC_output_PC5);
			double expected_power = calculate_power(digitized_adc_output_PC0, debug_COIL_CURRENT, debug_PWM_LIVE_TIME, debug_PWM_PERIOD);
			
			// only read from the buffer when RX is complete...
			if(RX_sequence_complete == true){
				// read from the buffer...
				uint8_t digit_req[3];
				usart_obtain_req(digit_req, usart_RX);
				uint8_t numerical_req = numerify_req(digit_req);
				// prepare report... (calculation etc...)
				// transmit report...
				RX_sequence_complete = false;
				
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
	/* USART on ATMEGA328P microcontroller is currently set to transmit and receive 8-bit ASCII characters across Master 
	(PC or equivalent system) and Slave (Linear Motor Controller) in JSON structure at a baud-rate of 9600.
	
	The input length to slave system is consistent in length of 38 characters (in ASCII) when fixed JSON is used. When slave 
	successfully receive 8-bit transmission from the master, USART Receive Complete flag is set.
	
	a. The received data (an ASCII character) is stored in an unsigned character buffer (usartRX).
	b. The index counter for the buffer is incremented for the upcoming data.
	c. On the 28th count (i.e. one sequence of master-slave communication is complete), reset the index position to overwrite the
	   first character of the buffer.
	 */
	ISR(USART_RX_vect){
		// Reception is complete... Need to find a way to extract information.
		if(usart_RX_index != JSON_FIXED_BUFFER_SIZE){				// Starting from index of zero, the buffer will be filled with 
			usart_RX[usart_RX_index] = UDR0;						// transmission up to 37th index.
		}
		else if (usart_RX_index == JSON_FIXED_BUFFER_SIZE){
			// If the index counter reaches 38, which is larger than the maximum length of the fixed JSON RX...
			usart_RX_index = 0;										 // Reset the Index back to zero (hence allow new sequence of RX)...
			usart_RX[usart_RX_index] = UDR0;						 // Record the first bit of the RX to the buffer...
			RX_sequence_complete = true;							 // Flag to main loop that one cycle of RX is complete.
		}
		usart_RX_index++;
	}
	
#endif

#ifdef ENABLE_PRINTF
	// Pulse Output is disabled due to unexpected behavior caused by long delays between global interrupt disabled and enabled.
	
#else

	/*** Timed-Pulse Modulation Unit Description ***/
	/*An 8-bit TIMER/COUNTER 2 (TC2) on ATMEGA328P microcontroller is currently set to overflow at approximately 1 millisecond.
	(1 ms)
	
	a. At every compare Match Flag A, the designated output pin will produce 5V (HIGH).
	b. At every Compare Match Flag B, the match counter variable (MATCH_COUNTER_T2) is incremented, indicating 
	   how many milliseconds has passed since the pin was activated.
	c. The live time of the output is controlled by setting the variable "PULSE_KILL_TIME" to a desired output duration: 
	   (i.e. when the match counter reaches the value of the "PULSE_KILL_TIME", the timed-pulse output is deactivated. For 
	   example, to produce a live-time of 6 ms, PULSE_KILL_TIME must be set to '6'.
	d. If the values for "PULSE_KILL_TIME" is not an integer, then take the ceiling of the number. (i.e. 3.34 -> 4).
	e. The total period of the signal between each polarity of timed-pulse output (i.e. PD6 and PB3) is controlled by setting 
	   the variable "PULSE_0_REACTIVATE_TIME" to the double of the desired period: for example, to produce a total "effective" 
	   period of 12 ms (i.e. 12 ms of PD6 + 12 ms of PB3), set "PULSE_0_REACTIVATE_TIME" to '24'.
	f. The variable "PULSE_2_ACTIVATE_TIME" must always be half of what was defined for "PULSE_0_REACTIVATE_TIME". If the value
	   for the variable is not an integer. Take the ceiling of the number (i.e. 3.34 -> 4).
	   
	/***WARNING***/
	/* The following conditions are critical to correct operation of the system. 
	a. When macro ENABLE_PRINTF is defined, Pulse Modulation will behave unexpectedly due to long delay between global interrupt 
	   disable and global interrupt enable caused by printf function. To ensure correct functionality of the Pulse Modulation, 
	   Macro ENABLE_PRINTF must first be undefined.*/
	
	ISR(TIMER2_COMPA_vect){
		if(MATCH_COUNTER_T2 == PULSE_0_START_TIME){
			PORTB &= ~(1 << PB3);						// Deactivate Output on PB3
			PORTD |= (1 << PD6);						// Activate Output on PD6
		}
		else if(MATCH_COUNTER_T2 == PULSE_2_START_TIME){
			PORTD &= ~(1 << PD6);						// Deactivate Output on PD6
			PORTB |= (1 << PB3);						// Activate Output on PB3
		}
		else if(MATCH_COUNTER_T2 == PULSE_0_REACTIVATE_TIME){
			PORTB &= ~(1 << PB3);						// Deactivate Output on PB3
			PORTD |=  (1 << PD6);						// Activate Output on PD6
			MATCH_COUNTER_T2 = PULSE_0_START_TIME;
		}
	}

	ISR(TIMER2_COMPB_vect){
		if(MATCH_COUNTER_T2 == PULSE_KILL_TIME){
			PORTD &= ~(1 << PD6);						// Deactivate Output PD6
		}
		if(MATCH_COUNTER_T2 == PULSE_KILL_TIME + PULSE_2_START_TIME){
			PORTB &= ~(1 << PB3);
		}
		MATCH_COUNTER_T2++;
	}
	
#endif

/*** Analog to Digital Conversion Complete Interrupt ***/
ISR(ADC_vect){
	
	/*** Analog-Ditical Converter Description ***/
	/* The Analog-Digital Converter on ATMEGA328P is currently set to take inputs from two channels: Channel 0 (located PD5) and 
	Channel 5 (located PD0).
	
	Channel 0 is assigned for coil voltage sensing.
	Channel 5 is assigned for coil current sensing. 
	
	a. Upon activation, analog-digital converter is initially set to receive input from Channel 0 (coil voltage). 
	b. At every completion of conversion, an interrupt flag is set. The output from ADC Register (ADC) is assigned to appropriate 
	   variables (raw_coil_voltage OR raw_coil_current).
	c. After ADC register values are assigned to a variable, channel switching occurs. Channels alternate between 0 to 5, and vice 
	   versa.
	d. A new conversion starts.
	e. The acquired ADC Register values are stored in the variable until next cycle of conversion is complete. It is passed down 
	   to main loop for fabrication.*/
	
	/*** WARNING ***/
	/* The following conditions are critical to correct operation of the system.
	a. As of 27.09.18. the feature to sample 24 points across the input waveform is not yet tested. */
	#ifdef ADC_DEBUG_MODE
	// Debugger Mode...
		if (ADC_next_channel == ADC_COIL_VOLTAGE_CHANNEL){
			raw_ADC_output_PC0 = ADC;
			debug_ADC_channel = ADC_next_channel;
			ADC_next_channel = ADC_COIL_CURRENT_CHANNEL;	// Next conversion is coil current shunt
			ADMUX &= 0xf0;									// Reset to Channel 0
			ADMUX |= 0x05;									// Set to Channel 5. (coil current shunt)
		}
		else if(ADC_next_channel == ADC_COIL_CURRENT_CHANNEL){
			raw_ADC_output_PC5 = ADC;
			debug_ADC_channel = ADC_next_channel;
			ADC_next_channel = ADC_COIL_VOLTAGE_CHANNEL;	// Next conversion is back to coil voltage shunt
			ADMUX &= 0xf0;									// Reset to Channel 0.
			ADMUX |= 0x00;									// Set to Channel 0. (coil voltage shunt)
		}
		ADCSRA |= (1 << ADSC);								// Start a new conversion...
			
	#else
		// Normal Operation Mode (w/o Hall Effect Sensors)	
		if (ADC_next_channel == ADC_COIL_VOLTAGE_CHANNEL){
			raw_ADC_output_PC0 = ADC;
			ADC_next_channel = ADC_COIL_CURRENT_CHANNEL;	// Next conversion is coil voltage shunt
			ADMUX &= 0xf0;									// Reset to Channel 0
			ADMUX |= 0x00;									// Set to Channel 0. (coil voltage shunt)
		}
		else if(ADC_next_channel == ADC_COIL_CURRENT_CHANNEL){
			raw_ADC_output_PC5 = ADC;
			ADC_next_channel = ADC_COIL_VOLTAGE_CHANNEL;	// Next conversion is back to coil current shunt
			ADMUX &= 0xf0;									// Reset to Channel 0.
			ADMUX |= 0x05;									// Set to Channel 5. (coil current shunt)
		}
		ADCSRA |= (1 << ADSC);								// Start a new conversion...
	#endif
	
}
// NOTE: PWM Output Gate for Timer 0 is PD6
// NOTE: PWM Output Gate for Timer 2 is PB3