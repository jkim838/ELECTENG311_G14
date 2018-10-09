/*
 * Comm_Setup.h
 *
 * Created: 9/23/2018 11:03:49
 * Author : Oliver K. jkim838 846548800
 * Revision 1.2.7
 *
 * Description:
 * This header file contains definitions for functions related to communication between master and slave system.
 *
 */

void usart_init(uint16_t UBRR);
void usart_transmit(uint8_t data);
void usart_TX_data(uint8_t communication_type);
int usart_printf(char var, FILE *stream);