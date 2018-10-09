/*
 * calculations.h
 *
 * Author: Oliver K jkim838 846548800
 *
 * Revision 1.1.5
 *
 * This header file contains definitions for functions related to Timer and PWM.
 *
 */

double calculate_length(volatile uint8_t* raw_hall_voltage_address);
double calculate_period(volatile uint8_t* raw_hall_voltage_address);
double calculate_flow_rate(uint8_t parameter1, uint8_t parameter2);
double calculate_power(double coil_voltage, double coil_current, double PWM_live_time, double cycle_period);
double calculate_voltage(double ADC_output_PC0);
double calculate_current(double ADC_output_PC5);
uint8_t numerify_req(uint8_t (*digit_req)[3]);