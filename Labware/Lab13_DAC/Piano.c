// Piano.c
// Runs on LM4F120 or TM4C123, 
// edX lab 13 
// There are four keys in the piano
// Daniel Valvano
// September 30, 2013

// Port E bits 3-0 have 4 piano keys

#include "Piano.h"
#include "..//tm4c123gh6pm.h"

# define C 520 
# define D 587 
# define E 659 
# define G 784



// **************Piano_Init*********************
// Initialize piano key inputs
// Input: none
// Output: none
void Piano_Init(void){volatile unsigned long delay; 
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOE; // activate port E
  delay = SYSCTL_RCGC2_R;
	
  GPIO_PORTE_AMSEL_R = 0x00;        // 3) disable analog on PE
  GPIO_PORTE_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PE3-0
  GPIO_PORTE_DIR_R = 0x00;          // 5) PE3-0 - 0 is input
  GPIO_PORTE_AFSEL_R = 0x00;        // 6) disable alt funct on PE7-0
  //GPIO_PORTE_PUR_R = 0x0F;          // enable pull-up on PE3-0
  GPIO_PORTE_DEN_R = 0x0F;          // 7) enable digital I/O on PE3-0
}
// **************Piano_In*********************
// Input from piano key inputs
// Input: none 
// Output: 0 to 15 depending on keys
// 0x01 is key 0 pressed, 0x02 is key 1 pressed,
// 0x04 is key 2 pressed, 0x08 is key 3 pressed
unsigned long Piano_In(void){
	long keyinput;
	long keypressed;
	keyinput = GPIO_PORTE_DATA_R&0x0F;
	
	switch (keyinput) {
		case 0x01:
			keypressed = C;
		  break;
		case 0x02:
			keypressed = D;
		  break;
		case 0x04:
			keypressed = E;
		  break;
		case 0x08:
			keypressed = G;
		  break;
		default:
			keypressed = 0;
  }
  return keypressed;
}
