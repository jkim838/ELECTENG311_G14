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
void usart_TX_data(uint8_t MOTOR_ID, uint8_t Current_FL, uint16_t numerical_req, double frequency, double expected_power, double coil_current, double coil_voltage, bool req_found, bool clear_error, bool error_collision, bool error_jammed);
int usart_printf(char var, FILE *stream);