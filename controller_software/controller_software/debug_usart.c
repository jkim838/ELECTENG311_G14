/*
 * debug_usart.c
 *
 * Created: 9/15/2018 17:30:46
 * Author: Oliver K jkim838 846548800
 *
 * Revision 1.1.5
 *
 * This script file contains definitions for functions used for debugging ADC
 *
 */

/*** Header Files ***/
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/*** Custom Header Files ***/
#include "debug_usart.h"

/*** Custom Macro Definitions ***/
#define F_CPU 16000000UL							// Define CPU Operational Clock
#define DATA_REG_IS_FULL !((1 << UDRE0) & UCSR0A)
#define ASCII_CONVERT 48							// Convert to ASCII Characters

/*** Delay Header ***/
#include <util/delay.h>


/*** Function Definitions ***/
void debug_usart_init(uint16_t UBRR){
	
	//Enable USART Transmitter
	UCSR0B |= (1 << TXEN0);
	//UCSR0B |= (1 << RXEN0);
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

void debug_usart_transmit(uint8_t data){
	
	while (DATA_REG_IS_FULL);		// Waits for the UDR register to be empty.
	UDR0 = data;					// Transmits the data.

}

void debug_usart_decompose(uint8_t *output, double input){
	
	input = input * 100;
	output[2] = (uint16_t)(input / 100);
	output[1] = (uint16_t)(input / 10) % 10;
	output[0] = (uint16_t)input % 10;
	
}

int usart_putchar_printf(char var, FILE *stream){

	if(var == '\n'){
		debug_usart_transmit('\r');
	}
	debug_usart_transmit(var);
	return 0;	

}