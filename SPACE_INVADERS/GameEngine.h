


 //Initialize, player, enemies, and bunkers
void Game_Init(void);

//Fire a regular shot from the player's ship if the number of shots on the screen are less than 4
//shots can pass through bunkers
//shots are launched from the center of the player ship
void RegShot_Fire(void);

//Fire a special shot object from the player's ship if the number of specialshots on the screen are less than 2
//Each special shot object launches two shots that move diagonal upwards and in opposite direction of each other
//shots can pass through bunkers
//shots are launched from the center of the player ship
void SpecShot_Fire(void);

//Move all the alive objects in the game according to each objects behavior
void Move_ActiveObjects(void);

//Detect all the collisions for the current fram and respond appropriately by, for example, turning on LEDS, printing explosions etc
void Check_Collisions(void);

//Draw the current game state 
void Draw_GameFrame(void); 

//Returns a number for SysTick period that is used to initialize SysTick interrupts with a frequency between 30-120Hz
//Number is returned such that frequency increases as number of killed enemies increases
//As more enemies are killed, game speed increases, making the game more difficult
unsigned long Set_Difficult(void);

//returns 1 if game over; 0 otherwise
unsigned char GameOverChecker(void);

//Output the frame for the Game Over State
void GameOverScreen(void);

