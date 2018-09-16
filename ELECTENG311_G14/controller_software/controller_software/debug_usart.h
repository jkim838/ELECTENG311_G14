/*
 * debug_usart.h
 *
 * Created: 9/15/2018 17:31:00
 * Author: Oliver K jkim838 846548800
 *
 * Revision 1.1.5
 *
 * This header file contains definitions for functions used to debug ADC.
 *
 */

/*** Global Variables ***/

/*** Function Prototypes ***/
void debug_usart_init(uint16_t UBRR);
void debug_usart_transmit(uint8_t data);
void debug_usart_decompose(uint8_t *output, double input);
int usart_putchar_printf(char var, FILE *stream);