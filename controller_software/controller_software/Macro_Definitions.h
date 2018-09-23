/*
 * Macro_Definitions.h
 *
 * Revision 1.2.4
 *
 * Description:
 * This header file contains prototypes for Macros which can be used widely between script files.
 *
 */

/*** Custom Macro Definitions ***/
/** Main Routine Definitions **/
#define F_CPU 16000000UL
/** ADC Routine Definitions **/
#define ADC_RESOLUTION 1024
#define ADC_REFERENCE_VOLTAGE 5
//#define ADC_ENABLE_AUTO_TRIGGER					// When enabled, the ADC will be automatically triggered as Timer1 overflows
#define ADC_COIL_VOLTAGE_CHANNEL 0
#define ADC_COIL_CURRENT_CHANNEL 5
/** Communication Routine Definitions **/
#define BAUD_RATE 9600
#define UBRR_VALUE F_CPU/16/BAUD_RATE - 1
#define DATA_REG_IS_FULL !((1 << UDRE0) & UCSR0A)
#define ASCII_CONVERT 48

/** Debugger Definitions **/
/* Main Debugger Definitions*/
#define TRANSMIT_DEBUG_MODE							// Enables displaying variable value by printf function. Output will be transmitted to PuTTy.
#ifdef TRANSMIT_DEBUG_MODE								// Warning: Ensure this macro is defined to use any of the sub-level debugger mode (e.g. ADC Debugger)	// otherwise the debugging feature will not function correctly.
	#define debug_UBRR F_CPU / 16 / BAUD_RATE -1
	#define PB_SW_IS_LOW ~PIND & (1 << PB7)				// Xplained Mini On-board Switch
#endif
#define ENABLE_DEBUGGING_PARAMETER						// Enables placeholder parameters
#define XPLAINED_MINI_LED_STROBE						// Enables LED Light on Xplained Mini Board (PB5) to strobe while the program is functional.
/* Timer Debugger Definitions */
#define TIMER_DEBUG_MODE
/* ADC Debugger Definitions */
#define ADC_DEBUG_MODE // Enables ADC Debugging Mode
#ifdef ADC_DEBUG_MODE
	#define debug_PWM_LIVE_TIME 0.006
	#define debug_PWM_PERIOD 0.02	// Period of the PWM. Not the period of the Motor
	#define debug_COIL_CURRENT 0.5	// Be mindful that this is a voltage level across the Current Shunt Resistor
#endif
/* Calculation Debugger Definitions */
#define CALCULATION_DEBUG_MODE
#ifdef CALCULATION_DEBUG_MODE
	#define VOLTAGE_SHUNT_A 1.6
	#define VOLTAGE_SHUNT_B 1
	#define CURRENT_RESISTOR_C 0.150
#else
	#define VOLTAGE_SHUNT_A	// Insert value for voltage shunt resistor A
	#define VOLTAGE_SHUNT_B // Insert value for voltage shunt resistor B
	#define CURRENT_RESISTOR_C // Insert value for current resistor C
#endif
