// template demonstration application for camera badge
// new apps need to be added to list in applist.h

#include "cambadge.h"
#include "globals.h"

// states used by this application

#define s_start 0
#define s_run 1
#define s_gameover 2
#define s_win 3

#define BALLSIZE 6
#define BALLXSTART  12
#define BALLYSTART  80
#define BALLXSTARTSPEED 200
#define BALLYSTARTSPEED 200

#define GAMECLOCKDIVISOR  24

#define PADDLESIZEX 36
#define PADDLESIZEY 3
#define PADDLESPEEDCHANGINGAREA 12
#define PADDLETOPY  124
#define PADDLESTARTX 32
#define PADDLECOLOR primarycol[1]
#define PADDLESPEEDSTART 3
#define SPEEDINCREMENT 20

#define BACKGROUNDCOLOR primarycol[0]
#define BALLCOLOR primarycol[2]

#define BRICKSIZEX  8
#define BRICKSIZEY  4
#define BRICKROWS   6
#define EMPTYROWSATTOP  2
#define TOPBRICK  EMPTYROWSATTOP*BRICKSIZEY
#define BOTBRICK  ((BRICKROWS+EMPTYROWSATTOP)*BRICKSIZEY)-1
#define BRICKSWIDE  16
#define BRICKSHIGH  6

typedef struct binPix {
  unsigned char monoPixel : 1;
} binaryPixel;


//Protoypes
void moveballX(char *xp, unsigned char x_dir);
void moveballY(char *yp, unsigned char y_dir);
unsigned int checkcollideX(unsigned char y, unsigned char x, char *x_dirp, binaryPixel *blocksarrayp);
unsigned int checkcollideY(unsigned char x, unsigned char y, unsigned char x_dir, char *y_dirp, char *speed_xp, unsigned char paddleX, binaryPixel *blocksarrayp);
unsigned char blockcollidecheck(signed char x, signed char y, binaryPixel *blocksp);
unsigned char paddleCollide(unsigned char ballX, unsigned char ballY, char *y_dirp, signed char paddleX);
unsigned char blocksRemaining(binaryPixel *blocks);
void newballspeed(unsigned char ballX, unsigned char paddleX, unsigned char dir_x, char *speed_xp);
void drawboard(void);
void erasebrick(unsigned char x, unsigned char y);

void moveballX(char *xp, unsigned char x_dir) {
  if (x_dir) ++*xp;
  else --*xp;
}

unsigned int checkcollideX(unsigned char x, unsigned char y, char *x_dirp, binaryPixel *blocksarrayp) {
  if (*x_dirp) {
    //Headed right
    if (x+BALLSIZE >= 127) {
      *x_dirp = 0;
    }
    else {
      unsigned char needrebound = 0;
      if (blockcollidecheck(x+BALLSIZE+1, y, blocksarrayp)) ++needrebound;
      if (blockcollidecheck(x+BALLSIZE+1, y+BALLSIZE, blocksarrayp)) ++needrebound;
      if (needrebound) {
        *x_dirp = 0;
        if (blocksRemaining(blocksarrayp) == 0) return s_win;
      }
    }
  }
  else {
    //Headed left
    if (x == 0) {
      *x_dirp = 1;
    }
    else {
      unsigned char needrebound = 0;
      if (blockcollidecheck(x+1, y, blocksarrayp)) ++needrebound;
      if (blockcollidecheck(x+1, y+BALLSIZE, blocksarrayp)) ++needrebound;
      if (needrebound) { 
        *x_dirp = 1;
        if (blocksRemaining(blocksarrayp) == 0) return s_win;
      }
    }
  }
  return s_run;
}

void moveballY(char *yp, unsigned char y_dir) {
  if (y_dir) ++*yp;
  else --*yp;
}

unsigned int checkcollideY(unsigned char x, unsigned char y, unsigned char x_dir, char *y_dirp, char *speed_xp, unsigned char paddleX, binaryPixel *blocksarrayp) {
  if (*y_dirp) {
    //Headed downward
    if (y+BALLSIZE >= 127) {
      *y_dirp = 0;
      return s_gameover; //Game Over
    }
    else if (paddleCollide(x, y, y_dirp, paddleX)) {
      //change speed
      //FIXME: horrible pointer bug in newballspeed -- speed_xp not being passed correctly
      newballspeed(x, paddleX, x_dir, speed_xp);
    }
    else {
      unsigned char needrebound = 0;
      if (blockcollidecheck(x, y+BALLSIZE+1, blocksarrayp)) ++needrebound;
      if (blockcollidecheck(x+BALLSIZE+1, y+BALLSIZE+1, blocksarrayp)) ++needrebound;
      if (needrebound) { 
        *y_dirp = 0;
        if (blocksRemaining(blocksarrayp) == 0) return s_win;
      }
    }
  }
  else {
    //Headed upward
    if (y == 0) {
      *y_dirp = 1;
    }
    else {
      unsigned char needrebound = 0;
      if (blockcollidecheck(x, y-1, blocksarrayp)) ++needrebound;
      if (blockcollidecheck(x+BALLSIZE+1, y-1, blocksarrayp)) ++needrebound;
      if (needrebound) {
        *y_dirp = 1;
        if (blocksRemaining(blocksarrayp) == 0) return s_win;
      }
    }
  }
  return s_run;
}

unsigned char blockcollidecheck(signed char x, signed char y, binaryPixel *blocksp) {
  if ((y>BOTBRICK) || (y<TOPBRICK)) return 0;
  unsigned char reducedX = x/BRICKSIZEX;
  unsigned char reducedY = (y/BRICKSIZEY)-EMPTYROWSATTOP;
  unsigned char idx = reducedX+(BRICKSWIDE*reducedY);
  if (blocksp[idx].monoPixel) {
    //We hit one so erase it
    blocksp[idx].monoPixel = 0;
    erasebrick(reducedX*BRICKSIZEX,(reducedY+EMPTYROWSATTOP)*BRICKSIZEY);
    return 1;
  }
  return 0;
}

unsigned char paddleCollide(unsigned char ballX, unsigned char ballY, char *y_dirp, signed char paddleX) {
  //Is ball moving downward?
  if (*y_dirp == 0) {
    //already going upward so ignore
    return 0;
  }
  
  //Is ball y value below paddletop-BALLSIZE?
  if ((ballY+BALLSIZE+1) == PADDLETOPY) {
    //Is ballx between paddle start stop?
    if (((ballX+BALLSIZE) < paddleX) || (ballX > (paddleX+PADDLESIZEX))) {
      //Ball has not collided
      return 0;
    }
    else {
      //Collision, bouncey is needed
      *y_dirp = 0;
      return 1;
    }
  }
  return 0;
}

unsigned char blocksRemaining(binaryPixel *blocks) {
  unsigned char boxcount = 0;
  unsigned char i;
  for (i=0; i<BRICKSWIDE*BRICKSHIGH; i++) {
    boxcount += blocks[i].monoPixel;
  }
  //printf(tabx6 taby0 "%d", boxcount);
  return boxcount;
}


void newballspeed(unsigned char ballX, unsigned char paddleX, unsigned char dir_x, char *speed_xp) {
  //Checks to see if we hit an outer edge and will change ball speed accordingly
  //printf(tabx0 taby1 "sp: %3d %3d %d %3d",ballX, paddleX, dir_x, (unsigned char) *speed_xp);
  if ((ballX+BALLSIZE) < (paddleX+PADDLESPEEDCHANGINGAREA)) {
    //Hit left speed changing area
    if (dir_x) {
      if ((unsigned char) *speed_xp < 255-SPEEDINCREMENT) *speed_xp += SPEEDINCREMENT;
    }
    else {
      if ((unsigned char) *speed_xp > SPEEDINCREMENT) *speed_xp -= SPEEDINCREMENT;    
    }
  }
     
  else if (ballX > (paddleX+PADDLESIZEX-PADDLESPEEDCHANGINGAREA)) {
    //Hit right speed changing area
    if (dir_x) {
      if ((unsigned char) *speed_xp > SPEEDINCREMENT) *speed_xp -= SPEEDINCREMENT;
    }
    else { 
      //*speed_xp = *speed_xp - 16;
      if ((unsigned char) *speed_xp < 255-SPEEDINCREMENT) *speed_xp += SPEEDINCREMENT;
    }
  }
  //else printf(tabx0 taby0 "mid");
}

void drawboard(void) {
  //FIXME
  plotblock(0,8,128,4,primarycol[3]);
  plotblock(0,12,128,4,primarycol[4]);
  plotblock(0,16,128,4,primarycol[5]);
  plotblock(0,20,128,4,primarycol[6]);
  plotblock(0,24,128,4,primarycol[7]);
  plotblock(0,28,128,4,primarycol[1]);
}

void drawpaddle(unsigned char x, unsigned char y) {
  plotblock(x ,y , PADDLESIZEX, PADDLESIZEY, PADDLECOLOR);
}

void erasepaddle(unsigned char x, unsigned char y) {
  plotblock(x ,y , PADDLESIZEX, PADDLESIZEY, BACKGROUNDCOLOR);
}

void drawball(unsigned char x, unsigned char y, unsigned short color) {
  plotblock(x, y, BALLSIZE, BALLSIZE, color);
}

void eraseball(unsigned char x, unsigned char y) {
  plotblock(x, y, BALLSIZE, BALLSIZE, BACKGROUNDCOLOR);
}

void erasebrick(unsigned char x, unsigned char y) {
  plotblock(x, y, BRICKSIZEX, BRICKSIZEY, BACKGROUNDCOLOR);
}

void movepaddle(char *paddleX, signed char paddleSpeed) {
  unsigned char tempPaddleX = *paddleX + paddleSpeed;
  if (tempPaddleX > 200) {
    //Arbitrary number of 200 is likely we subtracted past zero on the left so don't move paddle
    return;
  }
  if ((tempPaddleX+PADDLESIZEX) > 127) {
    //We've reached the right side of the screen so don't move paddle
    return;
  }
  erasepaddle(*paddleX, PADDLETOPY);
  *paddleX += paddleSpeed;
  drawpaddle(*paddleX, PADDLETOPY);
}

char* breakout(unsigned int action)
{
  static unsigned int state;
  static unsigned char gameclock;
  static unsigned char moveclockX;
  static unsigned char moveclockY;

  static signed char x;
  static signed char y;
  static unsigned char x_dir;
  static unsigned char y_dir;
  static unsigned char speed_x;
  static unsigned char speed_y;

  static binaryPixel blocks[BRICKSWIDE*BRICKSHIGH];
  
  static signed char paddleX;
  
  switch(action) {
     case act_name : return("BREAKOUT");
     case act_help : return("Simple game \nof Breakout");  
     case act_init :
         // add any code here that needs to run once at powerup - e.g. hardware detection/initialisation      
            return(0);   
     
     case act_powerdown :
          // add any code here that needs to run before powerdown
         return(0);
         
     case act_start :  
         // called once when app is selected from menu
         state=s_start;
         return(0);         
    } //switch
 
 //if (action!=act_poll) return(0);
 
 //if(!tick) return(0);
 
 switch(state) {
     case s_start :
       printf(cls);
       x = BALLXSTART;
       y = BALLYSTART;
       x_dir = 1;
       y_dir = 0;
       speed_x = BALLXSTARTSPEED;
       speed_y = BALLYSTARTSPEED;
       paddleX = PADDLESTARTX;
       gameclock = 0;
       moveclockX = 0;
       moveclockY = 0;
       
       unsigned char i;
       for (i=0; i<96; i++) blocks[i].monoPixel = 1;
       drawboard();
       drawpaddle(paddleX, PADDLETOPY);
       drawball(x, y, BALLCOLOR);
       state=s_run;
       break;
       
     case s_run :         
       //printf(tabx6 taby0 whi "X %5d Y %5d",accx,accy);
       //printf(tabx6 taby0 whi "%5d",gameclock);
        
        if (tick) {
           if (accx >= 1000) movepaddle(&paddleX, (unsigned char) (accx/1000)); //Right
           else if (accx <= -1000) movepaddle(&paddleX, abs((unsigned char) (accx/1000))); //Left
        }
        
        if (++gameclock > GAMECLOCKDIVISOR) {
          gameclock = 0;
          ++moveclockX;
          ++moveclockY;
        }
        
       
       //Timer ready to move X
        if (moveclockX > speed_x) {
          moveclockX = 0;
        
          eraseball(x, y); //Erase previous position
          moveballX(&x, x_dir);
          state = checkcollideX(x, y, &x_dir, blocks);
          drawball(x, y, BALLCOLOR); //Draw new position        
        }
        
        if (state == s_win) break;
        
        if (moveclockY > speed_y) {
          moveclockY = 0;
          
          //Move the ball on Y axis
          eraseball(x, y);
          moveballY(&y, y_dir);
          state = checkcollideY(x, y, x_dir, &y_dir, &speed_x, paddleX, blocks);
          drawball(x, y, BALLCOLOR); //Draw new position
        }
        
        break;
 
     case s_gameover : 
       printf(tabx6 taby6 whi "Game Over");
       if(butpress) state=s_start;
       break;
       
     case s_win:
       printf(tabx6 taby6 whi "You Win!");
       if(butpress) state=s_start;
       break;
 
 } // switch state
 
 if(butpress & but1) state=s_start;  // clear screen & restart
 if(butpress & but2) {
   //Left;
 }
 if (butpress & but3) {
   //Right;
 }
 if(butpress & powerbut) return(""); // exit with nonzero value to indicate we want to quit

 return(0);
 
}