/* Header files */

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "interrupt.h"
#include <math.h>




/* ------------ Subroutines List ------------ */
/* ------------------------------------------ */

/* ----------- Configs ----------- */
/* ------------------------------- */
void configIO();
void configInterrupts();
void configTimers();
void configPWM();
void configADC();

/* --------- System Ctrls --------- */
/* -------------------------------- */
void beltCtrl(char sysSTATE);		// STATE = run, stop, pause Button, RampDown Button
void DCMotorCtrl();
void stepperCtrl(int current_bin, int sort_bin)	
void stepperInit();				// Init by HE sensor

/* --------- Timers Calling --------- */
/* -------------------------------- */
void mTimer(int delay);			// timer for delay


/* -------------- Variables List -------------- */
/* -------------------------------------------- */

/* Motors */
// For DC motor
volatile unsigned char dutyCycle = 0x80; // set PWM = 50%
volatile unsigned char DC_cw = 0x02;		// DC clockwise
volatile unsigned char DC_ccw = 0x04		// DC counter-clockwise
volatile unsigned char DC_vccstop = 0x00  	// vcc stop DC motot

// For stepper motor
volatile unsigned char stepPORTA[4] = [s1, s2, s3, s4];
volatile unsigned char Steps[4] = [-90, 0, 90, 180]; // to do
volatile unsigned char ObjectReciv;


/* Sensors */
// For optical sensor IO
volatile unsigned int Count_OptIO = 0x00;
volatile unsigned int EnterFlagl;
// For inductive sensor IN
volatile unsigned int Count_IndIN = 0x00;
// For optical sensor OR
volatile unsigned int Count_OptOR = 0x00;

// For reflective sensor RL ADC
volatile unsigned int Count_RefRL;
volatile unsigned char ADC_resultH;
volatile unsigned char ADC_resultL;
volatile unsigned int ADC_result;
volatile unsigned int ADC_result_Flag;
volatile unsigned int Reflectness; // 16_bit to save 10-bit ADC reflectness

// For optical sensor EX
volatile unsigned int Count_OptEX;
volatile unsigned int ExitFlag;
// For Hall Effect sensor HE
volatile unsigned int HallFlag;

/* System State */
volatile unsigned char sysSTATE = ['run0','pause1','stop2','RampDown3'];

/* Cylinder Info Storage */
// For reflective values of different materials
// AL = 0-250;	STL = 251-600;  WhPLT = 601-950;	BlPLT = 951-1024;
#define AL_MAX = 250;
#define STL_MIN = 251;
#define STL_MAX = 600;
#define WPL_MIN = 601;
#define WPL_MAX = 950;
#define BPL_MIN = 951;


typedef struct cylinderMATERIAL{
	unsigned int refl;
	int inductive;		// 0 non ferrous, 1 ferrous
	int category;		// AL = 0; STL = 1; WPL = 2; BPL = 3; UNKnown = 4;

};
volatile cylinderMATERIAL CylindersINFO[48];



/* -------------- Main Loop -------------- */
/* --------------------------------------- */
/* --------------------------------------- */
/* --------------------------------------- */
int main()
{
	/* user parameters define here */




	cli();			// Disable all interrupts
	void configIO();
	void configInterrupts();
	void configTimers();
	void configPWM();
	void configADC();
	sei();			// enable all interrupts

	/* DC and Stepper Initialization here */
	stepperInit();


	// Start Polling 
	while(1){
		/* Sorting code here */
	}

	return 0;
}





/* LIGNE */






/* --------------- Motors Ctrl ------------ */
/* ---------------------------------------- */

void DCMotorCtrl(sysSTATE){
	switch (sysSTATE){
		case 0:
			PORTB = DC_cw;
			break;
		case 1:
			PORTB = DC_vccstop;
			break;
		case 2:
			buttonPressSTOP();
			break;
		case 3:
			buttonPressRAMPDOWN();
			break;
	}
}



/* ------------- Configurations ----------- */
/* ---------------------------------------- */

// Fini Mar 29, 3.25PM
void configIO(){
	/* IO Ports Definition */
	DDRA = 0xff;	// PORTA output, Stepper motor drive
	DDRB = 0xff;	// PORTB output, DC motor drive
	DDRC = 0xff;	// PORTC output, LEDs debugging
	DDRD = 0x00;	// PORTD[0,3] input, interrupts
	DDRE = 0x00;	// PORTE[4,7] input, interrupts
	DDRF = 0x00;	// PORTF1 Reflective ADC interupt
}


void configPWM(int duty_cyc){
	TCCR0A |= _BV(WGM00) | _BV(WGM01) | _BV(COM0A1);
	OCR0A = duty_cyc;
	PORTB = dc_clockwise;
}

// la Fini, Mar 29, 3PM
void configADC(){
	ADCSRA |= _BV(ADEN); 							// enable adc
	ADCSRA |= _BV(ADIE);							// enable interrupt of adc
	ADCSRA |= (_BV(ADPS2) | _BV(ADPS0));			// adc scaler division factor 32
	//ADCSRA |= _BV(ADSC)		// for the first conversion
	
	ADMUX |= _BV(ADLAR);							// set 10bit ADC value structure, ADCH[7,0] = ADC[9,2], ADCL[7,6] = ADC[1,0]
	ADMUX |= _BV(REFS0); 			// Vcc 3.3v Voltage reference with external capacitor on AREF pin
	ADMUX |= _BV(MUX0);								// channel select, ADC1

	//ADMUX = 0B11100001;		// gen shang mian de san hang dai ma yi yang

}

// la Fini, Mar 29, 3PM
void configInterrupts(){
	EIMSK |= 0b01111111;					// Enable INT0-6

	// Rising edge: INT2 & INT 6
	EICRA |= (_BV(ISC20) | _BV(ISC21));		// INT2 rising edge
	EICRB |= (_BV(ISC60) | _BV(ISC61));		// INT6 rising edge

	// Falling edge: INT0,1,3,4,5
	EICRA |= _BV(ISC01);					// INT0 falling edge
	EICRA |= _BV(ISC11);					// INT1 falling edge
	EICRA |= _BV(ISC31);					// INT3 falling edge

	EICRB |= _BV(ISC41);					// INT4 falling edge
	EICRB |= _BV(ISC51);					// INT5 falling edge

}




/* ----------- Des for interrupts --------- */
/* ---------------------------------------- */
// For ADC conversion, RL, PF1, INTADC, la Fini Mar 29, 3.25PM
ISR(ADC_vect){					// ADC interrupt for reflecness conversion , PF1
	

	ADC_resultH = ADCH;
	ADC_resultL = ADCL;

	ADC_resultH = (ADC_resultH & 0b11111111);
	ADC_resultL = (ADC_resultL & 0b11000000);
	ADC_resultL = (ADC_resultL >> 6);

	ADC_result = ADC_resultH;
	ADC_result = (ADC_result << 2);

	ADC_result |= ADC_resultL;

	//Reflectness = ADC_result

	ADC_result_Flag = 1;
}


// For optical 1, IO, PD0, INT0
ISR(INT0_vect){
	Count_OptIO++;		// optical 1, PD0
}

// For inductive, IN, PD1, INT1
ISR(INT1_vect){
	Count_IndIN++;				// inductive, PD1, for metal cylinders, for falling edge trigger
}


// For optical 2, OR, PD2, INT2, la Fini
ISR(INT2_vect){					//  optical 2, PD2 
	Count_OptOR++;
	ADCSRA|= _BV(ADSC); 		// rising on INT2, start ADC conversion

}

// For optical 3, EX, PD3, INT3
ISR(INT3_vect){
	Count_OptEX++;
}

// For press buttom low to stop/resume the belt, PE4, INT4
ISR(INT4_vect){
	DCMotorCtrl("pause");		// Vcc stop the dc motor, PE4 for falling edge press button
}


// For Hall effect sonsor under the stepper, PE5, INT5
ISR(INT5_vect){
	stepperStop();
	stepperInitFlag = 1;
	stepperPosition = 0x00;
}

// For press button high, PE6
ISR(INT6_vect){
	DCMotorCtrl();
}




/* --------- Timers Calling --------- */
/* -------------------------------- */
// For timer interrupt
ISR(TIMER1_OVF_vect){
	TIFR1 |= 0x01;
}

