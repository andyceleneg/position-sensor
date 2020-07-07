// ADC.c
// Runs on LM4F120/TM4C123
// Provide functions that initialize ADC0 to be triggered by
// software and trigger a conversion, wait for it to finish,
// and return the result.
// Andreina Granados
// May 7, 2020


#include "tm4c123gh6pm.h"
#include "ADC.h"


// This function should initialize ADC sampling at the lowest possible rate
// use a sequencer and step of your choice - currently initialized to PE4 Ain9, Sequencer 3
// Restrictions: Do not use PORT D for ADC input and only use sequencer 3 - causes channel conflicting error during simulation
void ADC_Init(void){ volatile unsigned long delay;
	SYSCTL_RCGCGPIO_R |= 0x010;		// 1) activate clock for Port E
	delay = SYSCTL_RCGCGPIO_R;		//		wait for clock to stabilize
	GPIO_PORTE_DIR_R &= ~0x10;		// 2) make PE4 input
	GPIO_PORTE_AFSEL_R |= 0x10;		// 3) enable alternate function on PE4
	GPIO_PORTE_DEN_R &= ~0x10;		// 4) disable digital I/O on PE4
	GPIO_PORTE_AMSEL_R |= 0x10;		// 5) enable analog function on PE4
	SYSCTL_RCGCADC_R |= 0x01;			// 6) activate ADC0
	delay = SYSCTL_RCGC2_R;				//		wait for clock to stabilize
	delay = SYSCTL_RCGC2_R;				//		wait for clock to stabilize
	delay = SYSCTL_RCGC2_R;				//		wait for clock to stabilize
	ADC0_PC_R = 0x01;							// 7) configure for 125K
	ADC0_SSPRI_R = 0x0123;				// 8) sequencer 3 has highest priority
	ADC0_ACTSS_R &= ~0x0008;			// 9) disable sample sequencer 3
	ADC0_EMUX_R &= ~0x5000;				// 10)sequencer 3 is a timed trigger
	ADC0_SSMUX3_R = (ADC0_SSMUX3_R&0xFFFFFFF0) + 9;			// 11)clear SS3 field & set channel Ain9 (PE4)   
	ADC0_SSCTL3_R = 0x0006;				// 12)no TS0 D0, yes IE0 END0
	ADC0_ACTSS_R |= 0x0008;				// 13)enable sample sequencer 3	
	
}

//------------ADC_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
unsigned long ADC_In(void){	unsigned long result;
	ADC0_PSSI_R = 0x0008;            // 1) initiate SS3
  while((ADC0_RIS_R&0x08)==0){};   // 2) wait for conversion done
  result = ADC0_SSFIFO3_R&0xFFF;   // 3) read result
  ADC0_ISC_R = 0x0008;             // 4) acknowledge completion
  return result;

}


unsigned long Convert(unsigned long input){ 
	unsigned long dout;											// y=0.4884x+0.2864
  dout = ((((125*input)+73))+128)/256;		// 8'b precision, +128 to correct rounding problem
	return dout;												
}


// **************Systick_Init*********************
// Initialize Systick periodic interrupts
// Input: Initial interrupt period
//           Units, Maximum and Minimum to be determined here
// Output: none
void Systick_Init(unsigned long period){
	NVIC_ST_CTRL_R = 0;						 // disable SysTick
	NVIC_ST_RELOAD_R = period-1; 	 // reload 24'b SysTick
	NVIC_ST_CURRENT_R = 0;				 // clear current
	NVIC_SYS_PRI3_R &= 0x1FFFFFFF; // set priority to 0: bits 31-29 for highest priority
	NVIC_ST_CTRL_R |= 0x07;				 // enable SysTick with interrupts
}


