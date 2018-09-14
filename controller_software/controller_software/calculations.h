/*
 * calculations.h
 *
 * Author: Oliver K jkim838 846548800
 *
 * Revision 1.1.4
 *
 * This header file contains definitions for functions related to Timer and PWM.
 *
 */

double calculate_length(volatile double* hall_voltage_address);
double calculate_period(volatile double* hall_voltage_address);
double calculate_flow_rate(uint8_t parameter1, uint8_t parameter2);
double calculate_power(double coil_voltage, double coil_current);