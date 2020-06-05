
#include "GameEngine.h"
#include "Nokia5110.h"
#include "Sprites.h"
#include "ADC.h"
#include "Random.h"

#define MAX_REG_SHOTS 4
#define MAX_SPEC_SHOTS 2
#define MAX_ROCKETS 5

unsigned long FrameCount=0;
unsigned long Distance; 
unsigned long ADCdata;  
unsigned long Score; //game score
unsigned char ShotsCount; //number of active shots on screen
unsigned char SpecShotsCount; //number of active special shots on screen
unsigned char SpecShotDectCheck; 
unsigned char RocketCount; //number of active enemyrocket on screen
unsigned char KilledEnemyCount;// (enemies on screen)
unsigned char RocketDelay; //used to create a delay between firing of successive Rockets


struct GameObject {
  unsigned long x;      // x coordinate
  unsigned long y;      // y coordinate
  unsigned char life;  // 0=dead, greater than 0 = alive
};         
typedef struct GameObject gameobject;

struct Player {
	gameobject GObj;
	const unsigned char *image; // a single pointer to image
																	
	unsigned char explode; //explode if > 0
};
typedef struct Player player;

struct Enemy {
	gameobject GObj;
	const unsigned char *image[2]; // two pointers to images
	unsigned char explode; //1 = draw explosion
	unsigned long hitBonus;																	
};
typedef struct Enemy enemy;

struct Bunker {
	gameobject GObj;
	const unsigned char *image[4]; //4 pointers to images
																		
};
typedef struct Bunker bunker;

/*Rockets fired by enemy invaders
A maximum of 5 Rockets will be allowed on the screen at the same time using counters
The speed would be set to make the Rockets travel vertically downwards*/
struct EnemyRocket {
	gameobject GObj;
	const unsigned char *image; // a single pointer to image
	unsigned long yspeed;																	
};
typedef struct EnemyRocket enemyrocket; 

/*Regular shots fired by the player ship
A maximum of four shots will be allowed on the screen at a time
The speed would be set to make the Rockets travel vertically upwards*/
struct Shot {
	gameobject GObj;
	const unsigned char *image; // a single pointer to image
	unsigned long yspeed;																	
};
typedef struct Shot shot; 

/*Special shots fired by the player ship
A maximum of 2 special shot objects will be allowed on the screen at a time
The x and y speeds will be set to make the shots travel diagonally
The speed would be set to make the Rockets travel vertically upwards*/
struct SpecShot {
	gameobject GObj1;
	gameobject GObj2;
	const unsigned char *image[2]; // 2 pointers to 2 images
	unsigned long hitBonus;
	unsigned long xspeed;	
	unsigned long yspeed;																	
};
typedef struct SpecShot specshot; 

player Player;
shot RegShots[MAX_REG_SHOTS];
specshot SpecShots[MAX_SPEC_SHOTS];
bunker Bunkers[2];
enemy Enemy[12];
enemyrocket Rockets[MAX_ROCKETS];



 
// Input: sample  12-bit ADC sample
// Output: 32-bit distance 
unsigned long ConvertToDistance(unsigned long sample){
   
	return ((750*sample) >> 10) + (429 >> 10); // returns a distance between 0 and 3 cm 
}

// access an enemy at random and launch a Rocket from its position
//or create a random value for RocketDelay
unsigned long RandomGenerator(unsigned long enemies){
  return ((Random()>>22)%enemies);  
}

void Game_Init(void){ 
	unsigned char i, j, k, l, m;
	Score = 0;
	ShotsCount = 0;
  SpecShotsCount = 0; 
  SpecShotDectCheck = 0;
  RocketCount = 0; 
  KilledEnemyCount = 0;
	RocketDelay = RandomGenerator(4);
	Player.GObj.x = 32;
	Player.GObj.y = 47;
	Player.image = PlayerShip0;
	Player.explode = 0;
	Player.GObj.life = 1;
	
	for(j=0;j<2;j++){
		Bunkers[j].GObj.x = (83-BUNKERW)*j;
    Bunkers[j].GObj.y = 47 - PLAYERH;
    Bunkers[j].image[0] = Bunker3;
    Bunkers[j].image[1] = Bunker2;
		Bunkers[j].image[2] = Bunker1;
		Bunkers[j].image[3] = Bunker0;
    Bunkers[j].GObj.life = 3;
	}
	
  for(i=0;i<12;i++){
		Enemy[i].GObj.life = 1;
		Enemy[i].explode = 0;
		if(i < 4){
			Enemy[i].GObj.x = 16*i;
			Enemy[i].GObj.y = ENEMY10H;
			Enemy[i].image[0] = SmallEnemy30PointA;
			Enemy[i].image[1] = SmallEnemy30PointB;
			Enemy[i].hitBonus = 30;
		}
		if((i < 8) && (i > 3)){
			Enemy[i].GObj.x = 16*(i-4);
			Enemy[i].GObj.y = 2*ENEMY10H;
			Enemy[i].image[0] = SmallEnemy20PointA;
			Enemy[i].image[1] = SmallEnemy20PointB;
			Enemy[i].hitBonus = 20;
		}
		if(i > 7){
			Enemy[i].GObj.x = 16*(i-8);
			Enemy[i].GObj.y = 3*ENEMY10H;
			Enemy[i].image[0] = SmallEnemy10PointA;
			Enemy[i].image[1] = SmallEnemy10PointB;
			Enemy[i].hitBonus = 10;
		}
   }
	
	 for(k = 0; k < MAX_ROCKETS; k++){
		 Rockets[k].GObj.life = 0;
	 }
	 for(l = 0; l < MAX_REG_SHOTS; l++){
		 RegShots[l].GObj.life = 0;
	 }
	 for(m = 0; m < MAX_SPEC_SHOTS; m++){
		 SpecShots[m].GObj1.life = 0;
		 SpecShots[m].GObj2.life = 0;
	 }
}

//Fire Rocket from an enemy if the number of Rockets on screen is less than 5
//Enemies chosen at random from the existing alive enemies
//This method called within RocketMove() method which is called every SysTick interrupt via Move_ActiveObjects() method

void EnemyRocketFire(void){unsigned char i, generate;
	if(RocketDelay){
		RocketDelay--;
		return;
	}
	RocketDelay = RandomGenerator(4); 
	for(i = 0; i < 12; i++){
		generate = RandomGenerator(2); //Random number which is either 0 or 1
		if(Enemy[i].GObj.life && (RocketCount < MAX_ROCKETS) && generate){
			//Rockets are 2 pixels wide
			//Enemies are 16 pixels wide
			Rockets[RocketCount].GObj.x = Enemy[i].GObj.x + 6; //set bottom left of Rocket at the center pixel along x axis of the enemy
			Rockets[RocketCount].GObj.y = Enemy[i].GObj.y;
			Rockets[RocketCount].GObj.life = 1;
			Rockets[RocketCount].image = Rocket0;
			Rockets[RocketCount].yspeed = 2;
			RocketCount++;
	
			return;
		}
	}

}

void RegShot_Fire(void){
	if(ShotsCount < MAX_REG_SHOTS){
			RegShots[ShotsCount].GObj.x = Player.GObj.x + 7; 
     //adding 7 makes Rocket image start at 8th pixel of player, making Rocket appear from player's center
			RegShots[ShotsCount].GObj.y = 47 - PLAYERH;
			RegShots[ShotsCount].GObj.life = 1;
			RegShots[ShotsCount].image = Rocket0;
			RegShots[ShotsCount].yspeed = 2;
			ShotsCount++;
	}
}

void SpecShot_Fire(void){
	if(SpecShotsCount < MAX_SPEC_SHOTS){
			SpecShots[SpecShotsCount].GObj1.x = Player.GObj.x + 6; //Player 18 pixels, shot is 4 pixels wide
																																//adding 6 makes shot image start at 8th pixel of player
																																// making shot appear from player's center
			SpecShots[SpecShotsCount].GObj1.y = 47 - PLAYERH;
			SpecShots[SpecShotsCount].GObj1.life = 1;
			SpecShots[SpecShotsCount].GObj2.x = Player.GObj.x + 6; 
			SpecShots[SpecShotsCount].GObj2.y = 47 - PLAYERH;
			SpecShots[SpecShotsCount].GObj2.life = 1;
			//shot0 will rise upwards and also move to the right
			//shot1 will rise upwards and also move to the left
			SpecShots[SpecShotsCount].image[0] =shot0;
			SpecShots[SpecShotsCount].image[1] =shot1;
			SpecShots[SpecShotsCount].hitBonus = 10;
			SpecShots[SpecShotsCount].xspeed = 2;
			SpecShots[SpecShotsCount].yspeed = 2;
			SpecShotsCount++;
	}
}

void CheckEnemyRegshotCollisions(void){unsigned char i, j;
	for(i = 0; i < 12; i++){		
		if(Enemy[i].GObj.life){
			for(j = 0; j < MAX_REG_SHOTS; j++){
					if((RegShots[j].GObj.life) && 
						!(((RegShots[j].GObj.x+RocketW) < Enemy[i].GObj.x) || (RegShots[j].GObj.x > (Enemy[i].GObj.x + ENEMY10W))) &&
						!((RegShots[j].GObj.y < (Enemy[i].GObj.y - ENEMY10H)) || ((RegShots[j].GObj.y - RocketH) > Enemy[i].GObj.y))){
								
							Score += Enemy[i].hitBonus;
							Enemy[i].GObj.life = 0;
							RegShots[j].GObj.life = 0;
							Enemy[i].explode = 1;
							ShotsCount--;
							KilledEnemyCount++;
							break;
					}
			}
		}
	}
}

void CheckEnemySpecshotCollisions(void){unsigned char i, j;
	for(i = 0; i < 12; i++){		
		if(Enemy[i].GObj.life){
			for(j = 0; j < MAX_SPEC_SHOTS; j++){
					if((SpecShots[j].GObj1.life) && 
						!(((SpecShots[j].GObj1.x+shotW) < Enemy[i].GObj.x) || (SpecShots[j].GObj1.x > (Enemy[i].GObj.x + ENEMY10W))) &&
						!((SpecShots[j].GObj1.y < (Enemy[i].GObj.y - ENEMY10H)) || ((SpecShots[j].GObj1.y - shotH) > Enemy[i].GObj.y))){
								
							Score += Enemy[i].hitBonus + SpecShots[j].hitBonus;
							Enemy[i].GObj.life = 0;
							SpecShots[j].GObj1.life = 0;
							Enemy[i].explode = 1;
							SpecShotDectCheck = 1;
							KilledEnemyCount++;
							break;
					}
						
					if((SpecShots[j].GObj2.life) && 
						!(((SpecShots[j].GObj2.x+shotW) < Enemy[i].GObj.x) || (SpecShots[j].GObj2.x > (Enemy[i].GObj.x + ENEMY10W))) &&
						!((SpecShots[j].GObj2.y < (Enemy[i].GObj.y - ENEMY10H)) || ((SpecShots[j].GObj2.y - shotH) > Enemy[i].GObj.y))){
								
							Score += Enemy[i].hitBonus + SpecShots[j].hitBonus;
							Enemy[i].GObj.life = 0;
							SpecShots[j].GObj2.life = 0;
							Enemy[i].explode = 1;
							SpecShotDectCheck = 1;
							KilledEnemyCount++;
							break;
					}
			}
		}
	}
}

void CheckBunkerperRocketCollision(void){unsigned char i, j;
		for(i = 0; i < 2; i++){		
		if(Bunkers[i].GObj.life){
			for(j = 0; j < MAX_ROCKETS; j++){
					if((Rockets[j].GObj.life) && 
						!(((Rockets[j].GObj.x+RocketW) < Bunkers[i].GObj.x) || (Rockets[j].GObj.x > (Bunkers[i].GObj.x + BUNKERW))) &&
						!((Rockets[j].GObj.y < (Bunkers[i].GObj.y - BUNKERH)) || ((Rockets[j].GObj.y - RocketH) > Bunkers[i].GObj.y))){
					
							Bunkers[i].GObj.life--;
							Rockets[j].GObj.life = 0;
							RocketCount--;
							if(Bunkers[i].GObj.life == 0){
								break;
							}
					}
			}
		}
	}
}

void CheckPlayerRocketCollision(void){ unsigned char j;
		if(Player.GObj.life){
			for(j = 0; j < MAX_ROCKETS; j++){
					if((Rockets[j].GObj.life) && 
						!(((Rockets[j].GObj.x+RocketW) < Player.GObj.x) || (Rockets[j].GObj.x > (Player.GObj.x + PLAYERW))) &&
						!((Rockets[j].GObj.y < (Player.GObj.y - PLAYERH)) || ((Rockets[j].GObj.y - RocketH) > Player.GObj.y))){
					
							Player.GObj.life = 0;
							Player.explode = 1;
							Rockets[j].GObj.life = 0;
							RocketCount--;
							break;
					}
			}
		}
}

void CheckRocketRegshotCollision(void){unsigned char i, j;
	for(i = 0; i < MAX_ROCKETS; i++){		
		if(Rockets[i].GObj.life){
			for(j = 0; j < MAX_REG_SHOTS; j++){
					if((RegShots[j].GObj.life) && 
						!(((RegShots[j].GObj.x+RocketW) < Rockets[i].GObj.x) || (RegShots[j].GObj.x > (Rockets[i].GObj.x + RocketW))) &&
						!((RegShots[j].GObj.y < (Rockets[i].GObj.y - RocketH)) || ((RegShots[j].GObj.y - RocketH) > Rockets[i].GObj.y))){
					
							Score += 1;
							Rockets[i].GObj.life = 0;
							RegShots[j].GObj.life = 0;
							ShotsCount--;
							RocketCount--;
							break;
					}
			}
		}
	}
}

void CheckRocketSpecshotCollision(void){unsigned char i, j;
	for(i = 0; i < MAX_ROCKETS; i++){		
		if(Rockets[i].GObj.life){
			for(j = 0; j < MAX_SPEC_SHOTS; j++){
					if((SpecShots[j].GObj1.life) && 
						!(((SpecShots[j].GObj1.x+shotW) < Rockets[i].GObj.x) || (SpecShots[j].GObj1.x > (Rockets[i].GObj.x + RocketW))) &&
						!((SpecShots[j].GObj1.y < (Rockets[i].GObj.y - RocketH)) || ((SpecShots[j].GObj1.y - shotH) > Rockets[i].GObj.y))){
					
							Score += 2;
							Rockets[i].GObj.life = 0;
							SpecShots[j].GObj1.life = 0;
							SpecShotDectCheck = 1;
							RocketCount--;
							break;
					}
						
						if((SpecShots[j].GObj2.life) && 
						!(((SpecShots[j].GObj2.x+shotW) < Rockets[i].GObj.x) || (SpecShots[j].GObj2.x > (Rockets[i].GObj.x + RocketW))) &&
						!((SpecShots[j].GObj2.y < (Rockets[i].GObj.y - RocketH)) || ((SpecShots[j].GObj2.y - shotH) > Rockets[i].GObj.y))){
					
							Score += 2;
							Rockets[i].GObj.life = 0;
							SpecShots[j].GObj2.life = 0;
							SpecShotDectCheck = 1;
							RocketCount--;
							break;
					}
			}
		}
	}
}

//Detect all the collisions for the current fram and respond 
//This method has to be called before the Move_ActiveObjects method
void Check_Collisions(void){
	CheckEnemyRegshotCollisions();
	CheckEnemySpecshotCollisions();
	CheckBunkerperRocketCollision();
	CheckPlayerRocketCollision();
	CheckRocketRegshotCollision();
	CheckRocketSpecshotCollision();	
}


/*Move player horizontally across the screen The x co-ordinate of the bottom left corner of the player ship can move 
from 0 to 64 on the x axis (player ship is 18 pixels wide and screen is 84 pixels)
The ConvertToDistance method in GameEngine.c returns a distance between 0 and 3 cm  
This distance was matched with pixel location on x axis, for the bottom left corner of player ship, 
using raw data from the simulation debugging  
The relationship was found to be BottomLeftX = Distance*0.0229 - 1.8 via a linear fit to data
0.0229 was approximated as 22/1024 (right shift by 10 is division by 1024) to get a pixel location accurate to 1 pixel
This pixel location varied from 0 to 64*/
void PlayerMove(void){
	Player.GObj.x = (ConvertToDistance(ADC0_In())*22) >> 10; 
}



void EnemyMove(void){ unsigned char i;
  for(i=0;i<12;i++){
    if(Enemy[i].GObj.x < 72){
      Enemy[i].GObj.x += 2; //Move all living enemies 2 pixels to the right
    }else{
      Enemy[i].GObj.x = 0; //reached end, start from left most end again
    }
  }
}

void RegshotMove(void){unsigned char i;
	for(i = 0; i < MAX_REG_SHOTS; i++){
		if(RegShots[i].GObj.life){
			if(RegShots[i].GObj.y <= RocketH ){
				RegShots[i].GObj.life = 0;
				ShotsCount--;	
			}
			else{
				RegShots[i].GObj.y -= RegShots[i].yspeed; 
			}
		}
	}
}

void SpecshotMove(void){
	unsigned char i;
	for(i = 0; i < MAX_SPEC_SHOTS; i++){
		if(SpecShots[i].GObj1.life){
			//Move shot0 image right
			if((SpecShots[i].GObj1.y > RocketH) && (SpecShots[i].GObj1.x < MAX_X-1-shotW)){
				SpecShots[i].GObj1.x += SpecShots[i].xspeed; 
				SpecShots[i].GObj1.y -= SpecShots[i].yspeed; 
			}
			else{
				SpecShotDectCheck = 1;
				SpecShots[i].GObj1.life = 0;
			}
		}
		
		if(SpecShots[i].GObj2.life){

			if((SpecShots[i].GObj2.y > RocketH) && (SpecShots[i].GObj2.x > shotW)){
				SpecShots[i].GObj2.x -= SpecShots[i].xspeed; 
				SpecShots[i].GObj2.y -= SpecShots[i].yspeed; 
			}
			else{
				SpecShotDectCheck = 1;
				SpecShots[i].GObj2.life = 0;
			}
	  }
		if((SpecShots[i].GObj1.life == 0) && (SpecShots[i].GObj2.life==0) && SpecShotDectCheck){
			SpecShotsCount--;
		}
		 SpecShotDectCheck = 0;
	}
}

void RocketMove(void){unsigned char i;
	for(i = 0; i < MAX_ROCKETS; i++){
		if(Rockets[i].GObj.life){
			if(Rockets[i].GObj.y >= (47)){
				Rockets[i].GObj.life = 0;
				RocketCount--;	
			}
			else{
				Rockets[i].GObj.y += Rockets[i].yspeed; 
			}
		}
	}
	EnemyRocketFire(); //create new Rocket if possible
}

void Move_ActiveObjects(void){
	PlayerMove();
	EnemyMove();
	RegshotMove();
	SpecshotMove();
	RocketMove();
}

void DrawPlayer(void){
	if(Player.GObj.life){
		Nokia5110_PrintBMP(Player.GObj.x, Player.GObj.y, Player.image, 0); 
	}
	else if(Player.explode){
		Player.explode = 0;
		Nokia5110_PrintBMP(Player.GObj.x, Player.GObj.y,  BigExplosion0, 0); 
	}
}

void DrawBunkers(void){unsigned char j;
	for(j=0;j<2;j++){
		Nokia5110_PrintBMP(Bunkers[j].GObj.x, Bunkers[j].GObj.y, Bunkers[j].image[Bunkers[j].GObj.life], 0); 
	}
}

void DrawEnemies(void){unsigned char i;
	 for(i=0;i<12;i++){
    if(Enemy[i].GObj.life > 0){
     Nokia5110_PrintBMP(Enemy[i].GObj.x, Enemy[i].GObj.y, Enemy[i].image[FrameCount], 0);
		}
		else{
			if(Enemy[i].explode){
					Nokia5110_PrintBMP(Enemy[i].GObj.x, Enemy[i].GObj.y, SmallExplosion0, 0);
				  Enemy[i].explode = 0;
			}
		}
  }
}

void DrawRegShots(void){unsigned char i;
	for(i=0;i<MAX_REG_SHOTS;i++){
    if(RegShots[i].GObj.life > 0){
     Nokia5110_PrintBMP(RegShots[i].GObj.x, RegShots[i].GObj.y, RegShots[i].image, 0);
		}
  }
}

void DrawSpecShots(void){unsigned char i;
	for(i=0;i<MAX_SPEC_SHOTS;i++){
    if(SpecShots[i].GObj1.life > 0){
     Nokia5110_PrintBMP(SpecShots[i].GObj1.x, SpecShots[i].GObj1.y, SpecShots[i].image[0], 0);
		}
		
		if(SpecShots[i].GObj2.life > 0){
     Nokia5110_PrintBMP(SpecShots[i].GObj2.x, SpecShots[i].GObj2.y, SpecShots[i].image[1], 0);
		}
  }
	
}

void DrawRockets(void){unsigned char i;
	for(i=0;i<MAX_ROCKETS;i++){
    if(Rockets[i].GObj.life){
     Nokia5110_PrintBMP(Rockets[i].GObj.x, Rockets[i].GObj.y, Rockets[i].image, 0);
		}
  }
}

void Draw_GameFrame(void){
  Nokia5110_ClearBuffer();
	DrawPlayer();
	DrawBunkers();
  DrawEnemies();
	DrawRegShots();
	DrawSpecShots();
	DrawRockets();
  Nokia5110_DisplayBuffer();      // draw buffer
  FrameCount = (FrameCount+1)&0x01; // 0,1,0,1,...
}

unsigned long Set_Difficult(void){

	//Increased returned period to make game run at a more playable speed
	return 2666666*4 - KilledEnemyCount*666666; //2666666*4 corresponds to period of SysTick interrupt with 7.5 Hz frequency
																						//12 (max number of killed enemies) * 666666 approximately equals (2666666*4)* (3/4)
																						
}

//returns 1 if game over; 0 otherwise
unsigned char GameOverChecker(void){
	if((KilledEnemyCount == 12) || (Player.GObj.life == 0))
		return 1;
	return 0;
}

//Output the frame for the Game Over State
void GameOverScreen(void){
	Nokia5110_Clear();
  Nokia5110_SetCursor(1, 0);
  Nokia5110_OutString("GAME OVER");
  Nokia5110_SetCursor(1, 1);
	if(KilledEnemyCount == 12){
		Nokia5110_OutString("You Won!");
		Nokia5110_SetCursor(1, 2);
		Nokia5110_OutString("Good job,");
	}
	else{
		Nokia5110_OutString("You Lost!");
		Nokia5110_SetCursor(1, 2);
		Nokia5110_OutString("Nice try,");
	}
  Nokia5110_SetCursor(1, 3);
  Nokia5110_OutString("Earthling!");
	Nokia5110_SetCursor(1, 4);
  Nokia5110_OutString("Score:");
  Nokia5110_SetCursor(7, 4);
  Nokia5110_OutUDec(Score);
}

