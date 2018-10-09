/*
 * main.c
 *
 * Created: 14/09/2018 8:12:55 AM
 * Author : Oliver K. jkim838 846548800
 * Revision 1.3.3
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
	#include "Comm_Setup.h"			// Contains functions to communicate between Master/Slave System.
#else
	#include "debug_usart.h"		// Contains functions to transmit variable values with USART Transmission
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


volatile uint8_t usart_RX_index = 0;
volatile unsigned char RX_buffer[JSON_FIXED_BUFFER_SIZE];			// The maximum assumed size of Rx buffer.
volatile bool RX_sequence_complete = false;							// Flow rate request will be read when the buffer is full


/*** Global Variable Definitions ***/
#ifdef TRANSMIT_DEBUG_MODE
	static FILE printf_stdout = FDEV_SETUP_STREAM(usart_printf, NULL, _FDEV_SETUP_WRITE);
#else
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
		uint8_t printf_value;
		stdout = &printf_stdout;
		usart_init(UBRR_VALUE);
	#else
	#endif
	
	sei();

    /* Main Loop */
    while (1) {
		
		// When buffer is filled with info.
		if(RX_sequence_complete){
			bool req_found = false;
			bool clear_error = false;
			uint16_t numerical_req;
			uint8_t digitized_req[3];
			//Verify Motor ID...
			if(RX_buffer[0] == '{' && RX_buffer[1] == '"' && RX_buffer[2] == '3' && RX_buffer[3] == '"' && RX_buffer[4] == ':'){
				// read through buffer contents
				for(uint8_t i = 0; i < JSON_FIXED_BUFFER_SIZE; i++){
						
					/*** Flow Rate Command Acquisition Description ***/
					/* This section of the code is only run when the correct Motor ID was provided from the master system.
					As the program reads through the entire character buffer, it tries to find the match between the 
					string q": from the full pattern "req:". Once this string pattern is found, then status "req_found"
					is set to true. Knowing the position of command value in the fixed JSON structure, the program will
					fetch and store each of its digits into an array "digitized_req".
					*/
						
					//check if "req" is present...
					if((RX_buffer[i]) == 'q' && (RX_buffer[i + 1]) == '"' && (RX_buffer[i + 2]) == ':' && (RX_buffer[i + 3]) == '"'){
						// req is present
						req_found = true;
						if(RX_buffer[i + 5] == '"'){
							digitized_req[0] = 0;
							digitized_req[1] = 0;
							digitized_req[2] = (RX_buffer[i + 4]);
						}
						else if(RX_buffer[i + 6] == '"'){
							digitized_req[0] = 0;
							digitized_req[1] = (RX_buffer[i + 4]);
							digitized_req[2] = (RX_buffer[i + 5]);
						}
						else if(RX_buffer[i + 7] == '"'){
							digitized_req[0] = (RX_buffer[i + 4]);
							digitized_req[1] = (RX_buffer[i + 5]);
							digitized_req[2] = (RX_buffer[i + 6]);
						}
					}
										
					if(RX_buffer[i] == 'r' && (RX_buffer[i + 1] == '"') && (RX_buffer[i + 2] == ':')&&(RX_buffer[i + 4] == 'e')&&(RX_buffer[i + 5] == 'w')){
						//clear error warning is present...
						clear_error = true;
					}
						
				}
					
				/*** Flow Rate Application Description ***/
				/* This section of the code is only run when the correct Motor ID was provided from the master system, and the
				program was able to acquire new flow rate command from the character buffer. There are five modes of operation:
				MAXIMUM, MINIMUM (NO OUTPUT), LOW FLOW, HIGH FLOW and OVERFLOW.
					
				MAXIMUM MODE: The timed-pulse output changes to maximum output mode when the flow rate command is equal to 255.
				MINIMUM MODE: The timed-pulse output changes to minimum output mode when the flow rate command is equal to zero.
				LOW FLOW MODE: The timed-pulse output changes to low flow mode when the flow rate command is between 1 to 177.
								The output is set to 10Hz. Its duty cycle (and hence stroke distance) changes as flow rate command
								becomes larger.
				HIGH FLOW MODE: The timed-pulse output changes to high output mode when the flow rate command is between 178 to
					            254. The output is set to 15Hz. Its duty cycle (and hence stroke distance) changes as flow rate
								command becomes larger.
				OVERFLOW MODE: The timed-pulse output changes to overflow mode when the flow rate command does not fall into any 
								of the specified region. The output is set to default mode (10Hz, 50% Duty Cycle).    
				*/
					
				// Finished reading through the buffer...
				// For testing purposes, print out the acquired "req" to the putty.
				numerical_req = (digitized_req[0]-'0') * 100 + (digitized_req[1]-'0') * 10 + (digitized_req[2]-'0');
				
				#ifdef TRANSMIT_DEBUG_MODE	
					for(uint8_t i = 0; i< 3; i++){
						usart_transmit(digitized_req[i]);
					}
					usart_transmit('\r');
				#endif

				// Apply new operating condition to the machine
				// Only change stroke frequency when REQ is acquired...
				if(req_found){
					// If command is "MAXIMUM OUTPUT"...
					if(numerical_req == TIMER_MAX){
						PULSE_0_REACTIVATE_TIME = 133;
						PULSE_2_START_TIME = 67;
						PULSE_KILL_TIME = 34;
					}
					// If command is "NO OUTPUT"...
					else if(numerical_req == TIMER_LOW){
						PULSE_0_REACTIVATE_TIME = 200;
						PULSE_2_START_TIME = 100;
						PULSE_KILL_TIME = 0;
					}
					// If command is between 1 to 177, set frequency to 10Hz, calculate the DUTY CYCLE equivalent...
					else if(numerical_req > TIMER_LOW && numerical_req != TIMER_LOW && numerical_req <= TIMER_LOW_FLOW){
						PULSE_0_REACTIVATE_TIME = 200;
						PULSE_2_START_TIME = 100;
						PULSE_KILL_TIME = ((double)(0.3 * numerical_req) * PULSE_0_REACTIVATE_TIME)/200;
					}
					// If command is between 178 to 254, set frequency to 15Hz, calculate the DUTY CYCLE equivalent...
					else if(numerical_req < TIMER_MAX && numerical_req != TIMER_MAX && numerical_req >= TIMER_HIGH_FLOW){
							
					}
					// If command is invalid, reset to default frequency...
					else{
						PULSE_0_REACTIVATE_TIME = 80;
						PULSE_2_START_TIME = 40;
						PULSE_KILL_TIME = 20;
					}
				}
				
				/*** Transmit Report Description ***/
						// Fetch Parameters...
				#ifdef ADC_DEBUG_MODE
					// try analog to digital conversion on the ADC, and display its output to the PuTTy.
					double coil_voltage = calculate_voltage(adc_digitize(raw_ADC_output_PC0));
					uint16_t coil_current = calculate_current(adc_digitize(raw_ADC_output_PC5)) * 1000;
					double expected_power = calculate_power(raw_ADC_output_PC0, debug_COIL_CURRENT, (PULSE_KILL_TIME / 1000), (0.5 * PULSE_0_REACTIVATE_TIME));
				#endif
				uint8_t MOTOR_ID = RX_buffer[2] - '0';
				uint8_t Current_FL = (200 * PULSE_KILL_TIME) / (0.3 * PULSE_0_REACTIVATE_TIME);
				double frequency = (1 / (0.5 * PULSE_0_REACTIVATE_TIME)) * 1000;
				printf("{");
				printf("\"%d\":", MOTOR_ID);
				printf("{");
				if(req_found){
					printf("\"mfc\":{\"req"":\"%d%d%d\",\"cur\":\"%d\"},\"ver:\"\"001.003.005"",", digitized_req[0]-'0',digitized_req[1]-'0',digitized_req[2]-'0', Current_FL);
				}
				else{
					printf("\"mfc\":{\"req"":\"%d%d%d\",\"cur\":\"%d\"},\"ver:\"\"001.003.005"",", 0,0,0, Current_FL);
				}
				printf("\"param\":{\"pwr\":\"%0.2fW\",\"freq\":\"%0.1fHz\",\"curr\":\"%dmA\",\"volt\":\"%0.2fV\"},", expected_power, frequency, coil_current, coil_voltage);
				if(!clear_error){
					printf("\"clr\":\"ew\",");
					printf("\"ew\":[\"cmprStalled\",\"blockedDuct\"]");	
				}
				else{
					printf("           "); // eleven spaces
					printf("                                  "); //31 spaces
				}
				printf("}");
				printf("}");
				
				// just to make the terminal look nicer...
				printf("\n");
				// When all the procedures with the sequence is complete...
				RX_sequence_complete = false;
			}
			else{
				// Wrong MOTOR ID is provided. Print Error Message.
				printf_value = RX_buffer[2];
				printf("VIOLATION: MOTOR ID '%d'\n", printf_value -'0');
				RX_sequence_complete = false;
			}
		}

		
		#ifdef XPLAINED_MINI_LED_STROBE
			PORTB ^= (1 << PB5);
			_delay_ms(100);
		#endif
		

    }
	
	cli();
	
	return 0;
}

/*** Interrupt Service Routine Definitions ***/

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
	if(usart_RX_index != JSON_FIXED_BUFFER_SIZE - 1){			// Starting from index of zero, the buffer will be filled with 
		RX_buffer[usart_RX_index] = UDR0;						// transmission up to 37th index.
		usart_RX_index++;
	}
	else if (usart_RX_index == JSON_FIXED_BUFFER_SIZE - 1){
		// If the index counter reaches 38, which is larger than the maximum length of the fixed JSON RX...
		RX_buffer[usart_RX_index] = UDR0;						 // Record the first bit of the RX to the buffer...
		RX_sequence_complete = true;							 // Flag to main loop that one cycle of RX is complete.
		usart_RX_index = 0;										 // Reset the Index back to zero (hence allow new sequence of RX)...
	}
		
}


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
		MATCH_COUNTER_T2 = PULSE_0_START_TIME;		// Reset counter to zero
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