// Lab6.c
// this lab creates a position measurment software that outputs the result on LCD
// The software samples analog input signal and use calibration to output the position
// Runs on LM4F120 or TM4C123
// Use the SysTick timer to request interrupts at a particular period.
// Andreina Granados 
// May 7, 2020 

#include "Nokia5110.h"
#include "PLL.h"
#include "ADC.h"
#include "tm4c123gh6pm.h"
 
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void WaitForInterrupt(void);  // low power mode	

// declare any global variable here
unsigned long Conversion;		// stores the value returned by the Convert function
unsigned long ADCMail;			// 
unsigned long ADCMail_old;
unsigned int ADCStatus;
unsigned int MSD;
unsigned int Dec;

// main program
int main(void){ 
	DisableInterrupts();
	PLL_Init(); 		// PLL.c adjusted to run the system at a bus frequency of 80MHz	
	ADC_Init();			// initializes PE4 Ain9 sequencer 3
	Nokia5110_Init();				// initializes LDC for displaying output
	Systick_Init(2666666);  // reload value for a 30Hz sample frequency
	EnableInterrupts();
	
	// initializing display to begin with an output of 0.0cm
	Nokia5110_OutUDec(0);
	Nokia5110_OutChar('.');
	Nokia5110_OutUDec(0);
	Nokia5110_OutString("cm");
	
	while(1){
		ADCStatus = 0;						// clear flag that signals new sample value
		WaitForInterrupt();				// SysTick_handler interrupt updates flag
		
		if (ADCStatus) {					// update LCD display IFF a different value is read
				Nokia5110_Clear();								// clear current display output
				Conversion = Convert(ADCMail);		// convert digital to corrected fixed-point
				MSD = Conversion/1000;						// store integer to the left of the decimal
				Dec = Conversion%1000;						// store integer to the right of the decimal
				Nokia5110_OutUDec(MSD);						// Nokia5110_XXXX() prints values to LCD 
				Nokia5110_OutChar('.');
				Nokia5110_OutUDec(Dec);
				Nokia5110_OutString("cm");
		}	
	}	
}


// Interrupt service routine
// Executed periodically, the actual period determined by the current Reload.
void SysTick_Handler(void){
	ADCMail = ADC_In();							// sample the ADC & store in global variable
	if(ADCMail_old != ADCMail) {		// only update if a new sample value is read
			ADCStatus = 1;							// flag signals when a new value is detected
			ADCMail_old = ADCMail;			// update the value last sampled to continue comparing
	}
	return;	
}

 
