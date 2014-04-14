// ***** 0. Documentation Section *****
// SwitchLEDInterface.c for Lab 8
// Runs on LM4F120/TM4C123
// Use simple programming structures in C to toggle an LED
// while a button is pressed and turn the LED on when the
// button is released.  This lab requires external hardware
// to be wired to the LaunchPad using the prototyping board.
// January 11, 2014

// Lab 8
//      Jon Valvano and Ramesh Yerraballi
//      November 21, 2013

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"

// ***** 2. Global Declarations Section *****


// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

// ***** 3. Subroutines Section *****

void InitPorts(void);
void LED_On(void);
void LED_Off(void);
void Delay100ms(unsigned long time);

// PE0, PB0, or PA2 connected to positive logic momentary switch using 10 k ohm pull down resistor
// PE1, PB1, or PA3 connected to positive logic LED through 470 ohm current limiting resistor
// To avoid damaging your hardware, ensure that your circuits match the schematic
// shown in Lab8_artist.sch (PCB Artist schematic file) or 
// Lab8_artist.pdf (compatible with many various readers like Adobe Acrobat).
unsigned long in,out;
int main(void){ 
//**********************************************************************
// The following version tests input on PE0 and output on PE1
//**********************************************************************
  TExaS_Init(SW_PIN_PE0, LED_PIN_PE1);  // activate grader and set system clock to 80 MHz
  InitPorts();
	
	
  EnableInterrupts();           // enable interrupts for the grader
  while(1){
    Delay100ms(1);
		
		in = GPIO_PORTE_DATA_R & 0x01; // in 0 if not pressed, 1 if pressed
		
		if (in && 0x01) { // switch is pressed
			GPIO_PORTE_DATA_R ^= 0x02; // toggle LED			
		} else {
			LED_On();
		}
  }  
}

// Make PE1 high
void LED_On(void){
  GPIO_PORTE_DATA_R |= 0x02;
}
// Make PE1 low
void LED_Off(void){
  GPIO_PORTE_DATA_R &= ~0x02;
}



void InitPorts(void) {
	volatile unsigned long delay;
	
	SYSCTL_RCGC2_R |= 0x10;           // Port E clock
  delay = SYSCTL_RCGC2_R;           // wait 3-5 bus cycles
  GPIO_PORTE_DIR_R = 0x02;         // PE1 output
  //GPIO_PORTE_DIR_R &= 0x01;        // PE0 input 
  GPIO_PORTE_AFSEL_R &= ~0x03;      // not alternative
  GPIO_PORTE_AMSEL_R &= ~0x03;      // no analog
  GPIO_PORTE_PCTL_R &= ~0x000000FF; // bits for PE1, PE0
  GPIO_PORTE_DEN_R |= 0x03;         // enable PE1, PE0
	
	GPIO_PORTE_DATA_R = 0x02; // set LED to on - PE1 = 1
}

void Delay100ms(unsigned long time){
  unsigned long i;
  while(time > 0){
    i = 1333333;  // this number means 100ms
    while(i > 0){
      i = i - 1;
    }
    time = time - 1; // decrements every 100 ms
  }
}
