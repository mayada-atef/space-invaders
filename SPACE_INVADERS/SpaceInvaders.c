// SpaceInvaders.c

// -------  Required Hardware I/O connections -------
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PE2/AIN1
// Slide pot pin 3 connected to +3.3V
// fire button connected to PE0
// special weapon fire button connected to PE1


#include "tm4c123gh6pm.h"
#include "Nokia5110.h"
#include "Random.h"
#include "ADC.h"
#include "GameEngine.h"
#include "TExaS.h"


// -------- Global Declarations Section ---------


unsigned char GameOverFlag;
unsigned char Semaphore = 0;
unsigned long PrevRegFire = 0;
unsigned long PrevSpecFire = 0;

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void SysTick_Init(unsigned long period); // Initialize SysTick interrupts
void StartScreen(void);
void Delay(unsigned long count); // time delay in 0.1 seconds
void Switchs_Init(void);
unsigned char Fire_Switch(void);
unsigned char SpecialFire_Switch(void);




int main(void){
	DisableInterrupts();
  TExaS_Init(SSI0_Real_Nokia5110_Scope);  // set system clock to 80 MHz
	Random_Init(1);
  Nokia5110_Init();
	SysTick_Init(2666666*4); // 2666666==> 30 Hz
	                         //Increased period by 4 for actual hardware to make the game run at a playable speed
  Nokia5110_ClearBuffer();
				StartScreen();
	Nokia5110_DisplayBuffer();      // draw buffer
	ADC0_Init();
	Game_Init();
	Switchs_Init();
	GameOverFlag = 0;
	EnableInterrupts();
	
	
  while(1){
		while(Semaphore==0){};
    Semaphore = 0;
		if(GameOverFlag){
			GameOverScreen();
			
		}
		else{

			Draw_GameFrame(); // update the LCD
		
		}	
		if((GameOverFlag == 0) && (GameOverChecker())){ //just detected game over
			Delay(2);//Delay 200ms
			GameOverFlag = GameOverChecker();
			SysTick_Init(2666666*4); // 2666666==> 30 Hz
			                          //Increased period by 4 for actual hardware to make the game run at a playable speed
		}
	}
}


void SysTick_Init(unsigned long period){
	NVIC_ST_CTRL_R = 0;         // disable SysTick during setup
  NVIC_ST_RELOAD_R = period-1;// reload value
  NVIC_ST_CURRENT_R = 0;      // any write to current clears it
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x20000000; // priority 1      
  NVIC_ST_CTRL_R = 0x0007;  // enable SysTick with core clock and interrupts
}
void SysTick_Handler(void){  // runs at frequency of SysTick interrupts
	//Game Engigine methods below
		
	if(GameOverFlag){
		if(Fire_Switch() || SpecialFire_Switch()){
			
			GameOverFlag = 0;
			Game_Init();
			
		}
	}
	else{
		Check_Collisions();
		Move_ActiveObjects();  
		if(Fire_Switch()){
			RegShot_Fire();
		
		}
		if(SpecialFire_Switch()){
			SpecShot_Fire();
			
		}
		SysTick_Init(Set_Difficult());
	}
  Semaphore = 1;
}




// Port E bits 1-0 are inputs
// fire button connected to PE0
// special weapon fire button connected to PE1

// Initialize switch inputs 
void Switchs_Init(void){ 
  volatile unsigned long  delay;
	//Clock for Port E already activated in ADC_Init which is called before this function in main
  /*SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOE;     // 1) activate clock for Port E
  delay = SYSCTL_RCGC2_R;           // allow time for clock to start	*/
	GPIO_PORTE_AMSEL_R &= ~0x03; // 3) disable analog function on PE1-0
  GPIO_PORTE_PCTL_R &= ~0x000000FF; // 4) enable regular GPIO on PE1-0
  GPIO_PORTE_DIR_R &= ~0x03;   // 5) inputs on PE1-0
  GPIO_PORTE_AFSEL_R &= ~0x03; // 6) regular function on PE1-0
  GPIO_PORTE_DEN_R |= 0x03;    // 7) enable digital on PE1-0
	
	
}

// Input from fire button (PE0)
// Input: none 
// Output: 0 or 1 depending on whether button was just pressed (positive logic)
unsigned char Fire_Switch(void){
	 unsigned char FireBool;
   if((GPIO_PORTE_DATA_R&0x01) && (PrevRegFire == 0)){ // just pressed
		 FireBool = 1;
	 }
	 else{
			FireBool = 0;
	 }
	 PrevRegFire = GPIO_PORTE_DATA_R&0x01;
	 return FireBool;
}


// Input from special weapons button (PE1)
// Input: none 
// Output: 0 or 1 depending on whether button was just pressed (positive logic)
unsigned char SpecialFire_Switch(void){
		unsigned char SpecFireBool;
   if((GPIO_PORTE_DATA_R&0x02) && (PrevSpecFire == 0)){ // just pressed
		 SpecFireBool = 1;
	 }
	 else{
		 SpecFireBool = 0;
	 }
	 PrevSpecFire = GPIO_PORTE_DATA_R&0x02;
	 return SpecFireBool;
}

void StartScreen(void){
	Nokia5110_Clear();
  Nokia5110_SetCursor(2, 2);
  Nokia5110_OutString("SPACE");
  Nokia5110_SetCursor(2, 4);
	Nokia5110_OutString("INVADERS");
	Nokia5110_SetCursor(0, 0);
	Delay(5);
	
	
}





void Delay(unsigned long count){ // 0.1*count
	unsigned long volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
}
