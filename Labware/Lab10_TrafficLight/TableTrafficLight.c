// ***** 0. Documentation Section *****
// TableTrafficLight.c for Lab 10
// Runs on LM4F120/TM4C123
// Index implementation of a Moore finite state machine to operate a traffic light.  
// Daniel Valvano, Jonathan Valvano
// November 7, 2013

// east/west red light connected to PB5
// east/west yellow light connected to PB4
// east/west green light connected to PB3
// north/south facing red light connected to PB2
// north/south facing yellow light connected to PB1
// north/south facing green light connected to PB0
// pedestrian detector connected to PE2 (1=pedestrian present)
// north/south car detector connected to PE1 (1=car present)
// east/west car detector connected to PE0 (1=car present)
// "walk" light connected to PF3 (built-in green LED)
// "don't walk" light connected to PF1 (built-in red LED)

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"

// ***** 2. Global Declarations Section *****

// port B output
#define CAR_LIGHT                   (*((volatile unsigned long *)0x400050FC)) // port B for out    // bits 5-0
#define GPIO_PORTB_OUT          (*((volatile unsigned long *)0x400050FC)) // bits 5-0
#define GPIO_PORTB_DIR_R        (*((volatile unsigned long *)0x40005400))
#define GPIO_PORTB_AFSEL_R      (*((volatile unsigned long *)0x40005420))
#define GPIO_PORTB_DEN_R        (*((volatile unsigned long *)0x4000551C))
#define GPIO_PORTB_AMSEL_R      (*((volatile unsigned long *)0x40005528))
#define GPIO_PORTB_PCTL_R       (*((volatile unsigned long *)0x4000552C))

// port E input

#define SENSOR                  (*((volatile unsigned long *)0x4002401C))	// port E for in   // bits 0,1,2

#define GPIO_PORTE_DIR_R        (*((volatile unsigned long *)0x40024400))
#define GPIO_PORTE_AFSEL_R      (*((volatile unsigned long *)0x40024420))
#define GPIO_PORTE_DEN_R        (*((volatile unsigned long *)0x4002451C))
#define GPIO_PORTE_AMSEL_R      (*((volatile unsigned long *)0x40024528))
#define GPIO_PORTE_PCTL_R       (*((volatile unsigned long *)0x4002452C))
	
// port F

#define PEDES_LIGHT                  (*((volatile unsigned long *) 0x400253FC))	// port E for in

#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

// ***** 3. Subroutines Section *****

void initB(void) {
	GPIO_PORTB_AMSEL_R &= ~0x3F; // 3) disable analog function on PB5-0
  GPIO_PORTB_PCTL_R &= ~0x00FFFFFF; // 4) enable regular GPIO
  GPIO_PORTB_DIR_R |= 0x3F;    // 5) outputs on PB5-0
  GPIO_PORTB_AFSEL_R &= ~0x3F; // 6) regular function on PB5-0
  GPIO_PORTB_DEN_R |= 0x3F;    // 7) enable digital on PB5-0
}

void initE(void) {
	GPIO_PORTE_AMSEL_R &= ~0x07; // 3) disable analog function on PE2-0
  GPIO_PORTE_PCTL_R &= ~0x000000FF; // 4) enable regular GPIO
  GPIO_PORTE_DIR_R &= ~0x07;   // 5) inputs on PE2-0
  GPIO_PORTE_AFSEL_R &= ~0x07; // 6) regular function on PE2-0
  GPIO_PORTE_DEN_R |= 0x07;    // 7) enable digital on PE2-0
}

void initF(void){ volatile unsigned long delay;
  //SYSCTL_RCGC2_R |= 0x00000020;     // 1) activate clock for Port F
  //delay = SYSCTL_RCGC2_R;           // allow time for clock to start
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0
  // only PF0 needs to be unlocked, other bits can't be locked
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog on PF
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PF4-0
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 in, PF3-1 out
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) disable alt funct on PF7-0
  GPIO_PORTF_PUR_R = 0x11;          // enable pull-up on PF0 and PF4
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital I/O on PF4-0
}

#define NVIC_ST_CTRL_R      (*((volatile unsigned long *)0xE000E010))
#define NVIC_ST_RELOAD_R    (*((volatile unsigned long *)0xE000E014))
#define NVIC_ST_CURRENT_R   (*((volatile unsigned long *)0xE000E018))
	
void SysTick_Init(void){
  NVIC_ST_CTRL_R = 0;               // disable SysTick during setup
  NVIC_ST_CTRL_R = 0x00000005;      // enable SysTick with core clock
}
// The delay parameter is in units of the 80 MHz core clock. (12.5 ns)
void SysTick_Wait(unsigned long delay){
  NVIC_ST_RELOAD_R = delay-1;  // number of counts to wait
  NVIC_ST_CURRENT_R = 0;       // any value written to CURRENT clears
  while((NVIC_ST_CTRL_R&0x00010000)==0){ // wait for count flag
  }
}
// 800000*12.5ns equals 10ms
void SysTick_Wait10ms(unsigned long delay){
  unsigned long i;
  for(i=0; i<delay; i++){
    SysTick_Wait(800000);  // wait 10ms
  }
}

// Linked data structure
struct State {
  unsigned long CarOut;
	unsigned long WalkOut;
  unsigned long Time;  
  unsigned long Next[8];}; 
typedef const struct State STyp;
	
#define allred0							0    //  lab notes started from 1. never should go into this state anyway. 	
#define allred1							1
#define westgreen						2
#define westyellow					3
#define southgreen					4
#define southyellow					5
#define walkgreen						6
#define dontwalkredon1			7
#define dontwalkredoff2			8
#define dontwalkredon3			9
#define dontwalkredoff4			10
#define dontwalkredon5			11
#define dontwalkredoff6			12
#define dontwalkredon7 			13
	
STyp FSM[14]={
 {0x24,0x02,1,{allred1,allred1,allred1,allred1,allred1,allred1,allred1,allred1}},
{0x24,0x02,1,{westgreen,westgreen,southgreen,westgreen,walkgreen,westgreen,southgreen,westgreen}},
{0x0C,0x02,60,{westgreen,westgreen,westyellow,westyellow,westyellow,westyellow,westyellow,westyellow}},
{0x14,0x02,60,{allred1,allred1,southgreen,southgreen,walkgreen,walkgreen,southgreen,southgreen}},
{0x21,0x02,60,{southgreen,southyellow,southgreen,southyellow,southyellow,southyellow,southyellow,southyellow}},
{0x22,0x02,60,{allred1,westgreen,allred1,westgreen,walkgreen,walkgreen,walkgreen,walkgreen}},
{0x24,0x08,60,{walkgreen,dontwalkredon1,dontwalkredon1,dontwalkredon1,walkgreen,dontwalkredon1,dontwalkredon1,dontwalkredon1}},
{0x24,0x02,20,{dontwalkredoff2,dontwalkredoff2,dontwalkredoff2,dontwalkredoff2,dontwalkredoff2,dontwalkredoff2,dontwalkredoff2,dontwalkredoff2}},
{0x24,0x00,20,{dontwalkredon3,dontwalkredon3,dontwalkredon3,dontwalkredon3,dontwalkredon3,dontwalkredon3,dontwalkredon3,dontwalkredon3}},
{0x24,0x02,20,{dontwalkredoff4,dontwalkredoff4,dontwalkredoff4,dontwalkredoff4,dontwalkredoff4,dontwalkredoff4,dontwalkredoff4,dontwalkredoff4}},
{0x24,0x00,20,{dontwalkredon5,dontwalkredon5,dontwalkredon5,dontwalkredon5,dontwalkredon5,dontwalkredon5,dontwalkredon5,dontwalkredon5}},
{0x24,0x02,20,{dontwalkredoff6,dontwalkredoff6,dontwalkredoff6,dontwalkredoff6,dontwalkredoff6,dontwalkredoff6,dontwalkredoff6,dontwalkredoff6}},
{0x24,0x00,20,{dontwalkredon7,dontwalkredon7,dontwalkredon7,dontwalkredon7,dontwalkredon7,dontwalkredon7,dontwalkredon7,dontwalkredon7}},
{0x24,0x02,20,{allred1,westgreen,southgreen,westgreen,allred1,westgreen,southgreen,westgreen}}
 };

unsigned long CurrentState;  // index to the current state 
unsigned long Input;

int main(void){ 

  TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210); // activate grader and set system clock to 80 MHz

  SYSCTL_RCGC2_R |= 0x32;      // 1) B E F

	initB();
	initF();
	initE();
	SysTick_Init();   // Program 10.2
	
  EnableInterrupts();
	CurrentState = 2;
  while(1){
     CAR_LIGHT = FSM[CurrentState].CarOut;  // set car lights
		 PEDES_LIGHT = FSM[CurrentState].WalkOut;  // set pedestrian lights
     SysTick_Wait10ms(FSM[CurrentState].Time);
     Input = SENSOR;     // read sensors
     CurrentState = FSM[CurrentState].Next[Input]; 
  }
}


