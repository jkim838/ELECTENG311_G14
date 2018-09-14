/*
 * adc_setup.c
 *
 * Created: 9/14/2018 18:17:46
 * Author: Oliver K jkim838 846548800
 *
 * Revision 1.1.4
 *
 * This header file contains definitions for functions related to Analog to Digital Converter.
 */ 

/*** Header Files ***/
#include <avr/io.h>
#include <stdint.h>

/*** Custom Header Files ***/
#include "adc_setup.h"


/*** Custom Macro Definitions ***/
#define ADC_RESOLUTION 1023			// define resolution for analog-to-digital converter.
//#define ADC_ENABLE_AUTO_TRIGGER

/*** Function Definitions ***/
void adc_init(){
	
	/** ADCMUX: ADC Multiplexer Selection Register **/
	/* Voltage Reference Selection */
	ADMUX &= ~(1 << REFS1);			// Use Vcc as reference voltage
	ADMUX |=  (1 << REFS0);
	/* ADC Left Adjust Result */
	ADMUX &= ~(1 << ADLAR);			// Right adjust ADC reading to ADCH
	/* Analog Channel Selection Bits */
	// Change input pin as specified by the data sheet...
	ADMUX &= ~(1 << MUX3);			// Channel 5 (Right Hall Effect Sensor): Input pin is PC5
	ADMUX |=  (1 << MUX2);
	ADMUX &= ~(1 << MUX1);
	ADMUX |=  (1 << MUX0);
	ADC_current_channel = 5;
	
	/** ADCSRA: ADC Control and Status Register A **/
	/* ADC Enable */
	ADCSRA |= (1 << ADEN);			// ADC Power 'On'
	/* ADC Auto Trigger */
	#ifdef ADC_ENABLE_AUTO_TRIGGER
		ADCSRA |= (1 << ADATE);			// Auto Trigger 'On'
	#endif
	/* ADC Interrupt Setting */
	//	ADCSRA &=~(1 << ADIF);
	ADCSRA |= (1 << ADIE);			// Interrupt 'On'
	/* Pre-scaler Setup */
	ADCSRA |= (1 << ADPS2);			// Pre-scaler Division factor to 64
	ADCSRA |= (1 << ADPS1);
	ADCSRA &=~(1 << ADPS0);
	
	/** ADCSRB: ADC Control and Status Register B **/
	/* ADC Auto Trigger Source */
	// NOTE: If ADATE is cleared, ADTS will have no effect
	#ifdef ADC_ENABLE_AUTO_TRIGGER
		ADCSRB &=~(1 << ADTS0);			// Timer/Counter 1 Overflow
		ADCSRB |= (1 << ADTS1);
		ADCSRB |= (1 << ADTS2);
	#endif

	/** ADC Begin **/
	ADCSRA |= (1 << ADSC);			// Start new conversion
	
}