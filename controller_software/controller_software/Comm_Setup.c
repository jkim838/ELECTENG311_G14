/*
 * Comm_Setup.c
 *
 * Created: 9/23/2018 11:03:36
 * Author : Oliver K. jkim838 846548800
 * Revision 1.2.7
 *
 * Description:
 * This script file contains definitions for functions related to communication between master and slave system.
 *
 */

/*** Header Files ***/
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
/*** Custom Header Files ***/
#include "Comm_Setup.h"
#include "Macro_Definitions.h"

void usart_init(uint16_t UBRR){
	
	// Enable USART Transmitter
	UCSR0B |= (1 << TXEN0);
	// Enable USART Receiver
	UCSR0B |= (1 << RXEN0);
	// Enable USART Receive Interrupt
	UCSR0B |= (1 << RXCIE0);
		
	// Sets the mode of the USART to be asynchronous.
	UCSR0C &= ~(1 << UMSEL01);
	UCSR0C &= ~(1 << UMSEL00);
		
	// Disables the parity bit.
	UCSR0C &= ~(1 << UPM01);
	UCSR0C &= ~(1 << UPM00);
		
	// Sets only one stop bit.
	UCSR0C &= ~(1 << USBS0);
		
	// Sets the data width to be 8 bits (UCSZ0[2..0] = 011).
	UCSR0B &= ~(1 << UCSZ02);
	UCSR0C |= ((1 << UCSZ01) | (1 << UCSZ00));
	// Sets the UBRR to be 103.
	UBRR0H = (UBRR >> 8);
	UBRR0L = UBRR;
	
}

void usart_transmit(uint8_t data){
	
	while (DATA_REG_IS_FULL);		// Waits for the UDR register to be empty.
	UDR0 = data;					// Transmits the data.
	
}

unsigned char usart_receive(){
	while(RX_INCOMPLETE);		// Wait for data to be received.
	return UDR0;
}

void usart_obtain_req(uint8_t (*target_array)[3], unsigned char(*usart_RX)[27]){
	
	uint8_t RX_index = 0;
	bool stop_loop = false;
	while (stop_loop == false){
		if(*usart_RX[RX_index] - '0'  == ':' && *usart_RX[RX_index + 1] - '0' == '"'){
			if(*usart_RX[RX_index + 3] - '0' == '"'){
				// Check if the RX sequence has one digits (i.e. sequence should have closer " after three indexes)
				*target_array[2] = 0;	// Assign zero to empty digit
				*target_array[1] = 0;	// Assign zero to empty digit
				*target_array[0] = usart_RX[RX_index + 2] - '0'; // convert back to numerals
			}
			else if(*usart_RX[RX_index + 4] == '"'){
				// Check if the RX sequence has two digits (i.e. sequence should have closer " after four indexes)
				*target_array[2] = 0;	// Assign zero to empty digit
				*target_array[1] = usart_RX[RX_index + 2] - '0'; // convert back to numerals
				*target_array[0] = usart_RX[RX_index + 3] - '0'; // convert back to numerals
			}
			else if(*usart_RX[RX_index + 5] - '0' == '"'){
				// Check if the RX sequence has three digits (i.e. sequence should have closer " after five indexes)
				*target_array[2] = usart_RX[RX_index + 2] - '0'; // convert back to numerals
				*target_array[1] = usart_RX[RX_index + 3] - '0'; // convert back to numerals
				*target_array[0] = usart_RX[RX_index + 4] - '0'; // convert back to numerals
			}
			stop_loop = true;
		}
	}
	
}

void usart_TX_data(uint8_t communication_type){
	
	// Slave to Master Communication
	usart_transmit('{'); usart_transmit('"'); usart_transmit('3'); usart_transmit('"');
	usart_transmit('{'); usart_transmit('"'); usart_transmit('m'); usart_transmit('f'); usart_transmit('c'); usart_transmit('"'); usart_transmit(':');
	usart_transmit('{'); usart_transmit('"'); usart_transmit('r'); usart_transmit('e'); usart_transmit('q'); usart_transmit('"'); usart_transmit(':');
	usart_transmit('"'); usart_transmit('X'); usart_transmit('X'); usart_transmit('X'); usart_transmit('"'); usart_transmit(',');
	usart_transmit('"'); usart_transmit('c'); usart_transmit('u'); usart_transmit('r'); usart_transmit('"'); usart_transmit(':');
	usart_transmit('"'); usart_transmit('X'); usart_transmit('X'); usart_transmit('X'); usart_transmit('"'); usart_transmit('}');
	usart_transmit('"'); usart_transmit('v'); usart_transmit('e'); usart_transmit('r'); usart_transmit('"'); usart_transmit(':');
	usart_transmit('"'); usart_transmit('X'); usart_transmit('X'); usart_transmit('X'); usart_transmit('"'); usart_transmit(',');
	usart_transmit('"'); usart_transmit('p'); usart_transmit('a'); usart_transmit('r'); usart_transmit('a'); usart_transmit('m'); usart_transmit('"'); usart_transmit(':'); usart_transmit('{');
	usart_transmit('"'); usart_transmit('p'); usart_transmit('w'); usart_transmit('r'); usart_transmit('"'); usart_transmit(':');
	usart_transmit('"'); usart_transmit('X'); usart_transmit('.'); usart_transmit('X'); usart_transmit('X'); usart_transmit('W'); usart_transmit('"'); usart_transmit(',');
	usart_transmit('"'); usart_transmit('f'); usart_transmit('r'); usart_transmit('e'); usart_transmit('q'); usart_transmit('"'); usart_transmit(':');
	usart_transmit('"'); usart_transmit('X'); usart_transmit('.'); usart_transmit('X'); usart_transmit('X'); usart_transmit('H'); usart_transmit('z'); usart_transmit('"'); usart_transmit(',');
	usart_transmit('"'); usart_transmit('c'); usart_transmit('u'); usart_transmit('r'); usart_transmit('r'); usart_transmit('"'); usart_transmit(':');
	usart_transmit('"'); usart_transmit('X'); usart_transmit('X'); usart_transmit('m'); usart_transmit('A'); usart_transmit('"'); usart_transmit(',');
	usart_transmit('"'); usart_transmit('v'); usart_transmit('o'); usart_transmit('l'); usart_transmit('t'); usart_transmit('"'); usart_transmit(':');
	usart_transmit('"'); usart_transmit('X'); usart_transmit('X'); usart_transmit('.'); usart_transmit('X'); usart_transmit('X'); usart_transmit('V'); usart_transmit('"'); usart_transmit('}'); usart_transmit(',');
	usart_transmit('"'); usart_transmit('c'); usart_transmit('l'); usart_transmit('r'); usart_transmit('"'); usart_transmit(':'); usart_transmit('"'); usart_transmit('e'); usart_transmit('w'); usart_transmit('"'); usart_transmit(',');
	usart_transmit('"'); usart_transmit('e'); usart_transmit('w'); usart_transmit('"'); usart_transmit(':'); usart_transmit('['); 
	usart_transmit('"'); usart_transmit('c'); usart_transmit('m'); usart_transmit('p'); usart_transmit('S'); usart_transmit('T'); usart_transmit('a'); usart_transmit('l'); usart_transmit('l'); usart_transmit('e'); usart_transmit('d'); usart_transmit('"'); usart_transmit(',');
	usart_transmit('"'); usart_transmit('p'); usart_transmit('i'); usart_transmit('s'); usart_transmit('t'); usart_transmit('o'); usart_transmit('n'); usart_transmit('C'); usart_transmit('o'); usart_transmit('l'); usart_transmit('l'); usart_transmit('i'); usart_transmit('s'); usart_transmit('i'); usart_transmit('o'); usart_transmit('n'); usart_transmit('"'); usart_transmit(']'); usart_transmit(',');
	usart_transmit('}');
	usart_transmit('}');

	
}