// template demonstration application for camera badge
// new apps need to be added to list in applist.h


#define PIXEL(x,y) cambuffer[y*dispwidth + x]



#include "cambadge.h"
#include "globals.h"
#include <math.h>
#include <string.h>
#include <float.h>

// states used by this application

#define s_start 0
#define s_run 1
#define s_freeze 2

const uint8_t sinlut[256] = {
    0x80,0x83,0x86,0x89,0x8c,0x8f,0x92,0x95,0x98,0x9b,0x9e,0xa2,0xa5,0xa7,0xaa,0xad,
    0xb0,0xb3,0xb6,0xb9,0xbc,0xbe,0xc1,0xc4,0xc6,0xc9,0xcb,0xce,0xd0,0xd3,0xd5,0xd7,
    0xda,0xdc,0xde,0xe0,0xe2,0xe4,0xe6,0xe8,0xea,0xeb,0xed,0xee,0xf0,0xf1,0xf3,0xf4,
    0xf5,0xf6,0xf8,0xf9,0xfa,0xfa,0xfb,0xfc,0xfd,0xfd,0xfe,0xfe,0xfe,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xfe,0xfe,0xfe,0xfd,0xfd,0xfc,0xfb,0xfa,0xfa,0xf9,0xf8,0xf6,
    0xf5,0xf4,0xf3,0xf1,0xf0,0xee,0xed,0xeb,0xea,0xe8,0xe6,0xe4,0xe2,0xe0,0xde,0xdc,
    0xda,0xd7,0xd5,0xd3,0xd0,0xce,0xcb,0xc9,0xc6,0xc4,0xc1,0xbe,0xbc,0xb9,0xb6,0xb3,
    0xb0,0xad,0xaa,0xa7,0xa5,0xa2,0x9e,0x9b,0x98,0x95,0x92,0x8f,0x8c,0x89,0x86,0x83,
    0x80,0x7c,0x79,0x76,0x73,0x70,0x6d,0x6a,0x67,0x64,0x61,0x5d,0x5a,0x58,0x55,0x52,
    0x4f,0x4c,0x49,0x46,0x43,0x41,0x3e,0x3b,0x39,0x36,0x34,0x31,0x2f,0x2c,0x2a,0x28,
    0x25,0x23,0x21,0x1f,0x1d,0x1b,0x19,0x17,0x15,0x14,0x12,0x11,0xf,0xe,0xc,0xb,
    0xa,0x9,0x7,0x6,0x5,0x5,0x4,0x3,0x2,0x2,0x1,0x1,0x1,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x2,0x2,0x3,0x4,0x5,0x5,0x6,0x7,0x9,
    0xa,0xb,0xc,0xe,0xf,0x11,0x12,0x14,0x15,0x17,0x19,0x1b,0x1d,0x1f,0x21,0x23,
    0x25,0x28,0x2a,0x2c,0x2f,0x31,0x34,0x36,0x39,0x3b,0x3e,0x41,0x43,0x46,0x49,0x4c,
    0x4f,0x52,0x55,0x58,0x5a,0x5d,0x61,0x64,0x67,0x6a,0x6d,0x70,0x73,0x76,0x79,0x7c,
};

void draw();
void readKeys();
void generateTextures();
void makepalette(int offset);

int counter = 0;
long timer = 0;
long fpsTimer = 0;
int fps = 0;
long frames = 0;

enum {
    SOLO, LEFTSIDE, RIGHTSIDE
} syncMode = SOLO;


//worldstate vars
double posX = 22.0, posY = 11.5; //x and y start position
double dirX = -1.0, dirY = 0.0; //initial direction vector
double planeX = 0.0, planeY = 0.66; //the 2d raycaster version of camera plane

double paletteOffset = 0;
int rxError = 0;

void printMode() {
    switch (syncMode) {
        default:
            syncMode = SOLO;
            printMode();
            break;
        case SOLO:
            printf(cls top butcol "EXIT" bot "                 Solo");
            break;
        case LEFTSIDE:
            printf(cls top butcol "EXIT" bot "                 Left");
            break;
        case RIGHTSIDE:
            printf(cls top butcol "EXIT" bot "                Right");
            break;
    }
}

int u2rxbyte() {
    
    T1CON = 0b1000000000000000;
    T1CONbits.TCKPS0 = 1;
    PR1 = 0xffff;
    TMR1 = 0;
    IFS0CLR = _IFS0_T1IF_MASK;
    
    while (!U2STAbits.URXDA) {
        if (U2STAbits.OERR) {
            U2STAbits.OERR = 0;
            return -1;
        } // UART overrun
        if (U2STAbits.FERR) {
            U2STAbits.FERR = 0;
            return -1;
        } // Framing error

        if (IFS0bits.T1IF)
            return -1;
        kickwatchdog;
    };
    return U2RXREG;
}

double receiveDouble() {
    double d;
    unsigned char *cp = (char*) &d;
    int i, c;
    for (i = 0; i < sizeof (double); i++) {
        c = u2rxbyte();
        if (c == -1)
            return DBL_MIN;
        *cp++ = c;
    }
    return d;
}

void sendDouble(double d) {
    unsigned char *c = (char*) &d;
    u2txbyte(*c++);
    u2txbyte(*c++);
    u2txbyte(*c++);
    u2txbyte(*c++);
}

txState() {
    //I hope this was easier than refactoring everything into a struct

    u2txbyte(0); //frame start
    sendDouble(posX);
    sendDouble(posY);
    sendDouble(dirX);
    sendDouble(dirY);
    sendDouble(planeX);
    sendDouble(planeY);
    sendDouble(paletteOffset);
    u2txbyte(0xff); //frame end
    rxError = 0;
    if (u2rxbyte() != 1) { //wait for other side to ack
        rxError = 1;
//        delayus(10000); //delay long enough for rx to timeout and reset
    }
    
}


#define eyeShift -0.2


rxState() {
    rxError = 1;
    //I hope this was easier than refactoring everything into a struct
    double rposX = 22.0, rposY = 11.5; //x and y start position
    double rdirX = -1.0, rdirY = 0.0; //initial direction vector
    double rplaneX = 0.0, rplaneY = 0.66; //the 2d raycaster version of camera plane

    if (u2rxbyte() != 0) return; //frame start
    if ((rposX = receiveDouble()) == DBL_MIN) return;
    if ((rposY = receiveDouble()) == DBL_MIN) return;
    if ((rdirX = receiveDouble()) == DBL_MIN) return;
    if ((rdirY = receiveDouble()) == DBL_MIN) return;
    if ((rplaneX = receiveDouble()) == DBL_MIN) return;
    if ((rplaneY = receiveDouble()) == DBL_MIN) return;
    if ((paletteOffset = receiveDouble()) == DBL_MIN) return;
    if (u2rxbyte() != 0xff) return; //frame end

    posX = rposX;
    posY = rposY;
    dirX = rdirX;
    dirY = rdirY;
    planeX = rplaneX;
    planeY = rplaneY;
    
    
    //shift eye position
//    rposX = rposX + rdirX * eyeShift;
//    rposY = rposY + rdirY * eyeShift;
    //(x,y) rotated left 90 deg is (-y, x) . pretend that we faced left, moved a bit, then restored direction
    rposX = rposX + -rdirY * eyeShift;
    rposY = rposY + rdirX * eyeShift;

    rxError = 0;
    u2txbyte(1); //send ack    
}

char* demoapp(unsigned int action) {
    static unsigned int state, colour;
    unsigned int x, y, i;

    switch (action) {
        case act_name: return ("TEST_APP");
        case act_help: return ("line1\nline2\nline3");
        case act_init:
            // add any code here that needs to run once at powerup - e.g. hardware detection/initialisation      
            return (0);

        case act_powerdown:
            // add any code here that needs to run before powerdown
            return (0);

        case act_start:
            // called once when app is selected from menu
            state = s_start;
            colour = 1;
            return (0);
    } //switch

    if (action != act_poll) return (0);

    // do anything that needs to be faster than tick here.

    
        if (!tick) return (0);
    timer += tick;

    if (timer >= fpsTimer + 50) {
        fps = (50 * frames) / (timer - fpsTimer);
        fpsTimer = timer;
        frames = 0;
//        makepalette(timer);
    }

    switch (state) {
        case s_start:
            monopalette(0, 255);
            generateTextures();
            syncMode = SOLO;
            //              printf(cls top butcol "EXIT" bot "Clear   Colour Freeze");
            printMode();
            state = s_run;


        case s_run:
            printf(tabx6 taby0 whi "%5dfps e:%d", fps, rxError);

            draw();

            dispimage(0, 8, dispwidth, dispheight - 16, img_mono | (syncMode == LEFTSIDE ? img_revscan : 0), cambuffer);
            frames++;

            switch (syncMode) {
                case LEFTSIDE:
                    rxState();
                    makepalette(paletteOffset);
                    break;
                case RIGHTSIDE:
                    paletteOffset += 4;
                    makepalette(paletteOffset);
                    readKeys();
                    txState();
                case SOLO:
                    paletteOffset += 4;
                    makepalette(paletteOffset);
                    readKeys();
            }

            if (butpress & but3) {
                syncMode++;
                printMode();
            }
            break;

        case s_freeze:
            if (butpress) state = s_run;
            break;

    } // switch state

    //    if (butpress & but1) state = s_start; // clear screen & restart
    //    if (butpress & but2) if (++colour == 8) colour = 1;





    if (butpress & powerbut) return (""); // exit with nonzero value to indicate we want to quit

    return (0);

}




/*
Copyright (c) 2004-2007, Lode Vandevenne

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#define screenWidth 128
#define screenHeight 112
#define mapWidth 24
#define mapHeight 24

#define texHeight 32
#define texWidth 32

#define TEXTURE_MEM_OFFSET (128*128)
#define TEXTURE_SIZE (texWidth*texHeight)

#define TEXTURE(t,x,y) cambuffer[TEXTURE_MEM_OFFSET + TEXTURE_SIZE * t + y*texWidth + x]


int worldMap[mapWidth][mapHeight] = {
    {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 7, 7, 7, 7, 7, 7, 7, 7},
    {4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 7},
    {4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7},
    {4, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7},
    {4, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 7},
    {4, 0, 4, 0, 0, 0, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 0, 7, 7, 7, 7, 7},
    {4, 0, 5, 0, 0, 0, 0, 5, 0, 5, 0, 5, 0, 5, 0, 5, 7, 0, 0, 0, 7, 7, 7, 1},
    {4, 0, 6, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 5, 7, 0, 0, 0, 0, 0, 0, 8},
    {4, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 1},
    {4, 0, 8, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 5, 7, 0, 0, 0, 0, 0, 0, 8},
    {4, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 5, 7, 0, 0, 0, 7, 7, 7, 1},
    {4, 0, 0, 0, 0, 0, 0, 5, 3, 5, 4, 0, 5, 6, 5, 5, 7, 7, 7, 7, 7, 7, 7, 1},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 6, 6, 6, 6, 1, 2, 3, 4, 5, 6, 7, 6},
    {8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {6, 6, 6, 6, 6, 6, 0, 6, 1, 2, 3, 0, 4, 5, 6, 7, 8, 6, 6, 6, 6, 6, 8, 6},
    {4, 4, 4, 4, 4, 4, 0, 4, 4, 4, 6, 0, 6, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3},
    {4, 0, 0, 0, 0, 0, 0, 0, 0, 4, 6, 0, 6, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2},
    {4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 2, 0, 0, 5, 0, 0, 2, 0, 0, 0, 2},
    {4, 0, 0, 0, 0, 0, 0, 0, 0, 4, 6, 0, 6, 2, 0, 0, 0, 0, 0, 2, 2, 0, 2, 2},
    {4, 0, 6, 0, 6, 0, 0, 0, 0, 4, 6, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 2},
    {4, 0, 0, 5, 0, 0, 0, 0, 0, 4, 6, 0, 6, 2, 0, 0, 0, 0, 0, 2, 2, 0, 2, 2},
    {4, 0, 6, 0, 6, 0, 0, 0, 0, 4, 6, 0, 6, 2, 0, 0, 5, 0, 0, 2, 0, 0, 0, 2},
    {4, 0, 0, 0, 0, 0, 0, 0, 0, 4, 6, 0, 6, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2},
    {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3}
};



double time = 0; //time of current frame
double oldTime = 0; //time of previous frame


void makepalette(int offset) {
    unsigned int i, d;
    uint8_t r, g, b;
    for (i = 0; i != 256; i++) {
        d = i + offset;
        r = (sinlut[d & 0xff] *i)>>8;
        g = (sinlut[(d+85) & 0xff] *i)>>8;
        b = (sinlut[(d+170) & 0xff] *i)>>8;
        palette[i] = rgbto16(r, b, g);
    }
    palette[0] = 0;
}

void generateTextures() {
    int x, y, t;
    for (x = 0; x < texWidth; x++) {
        for (y = 0; y < texHeight; y++) {

            TEXTURE(0, x, y) = ((x + y)&0x7) * 8;
            TEXTURE(1, x, y) = abs(x - y)*4;
            TEXTURE(2, x, y) = abs(y - x)*4;
            TEXTURE(3, x, y) = (x + y)*4;
            TEXTURE(4, x, y) = x*y;
            TEXTURE(5, x, y) = abs(sin((x - texWidth) / 6.0) * cos((y - texHeight) / 6.0) * 255);
            TEXTURE(6, x, y) = (x ^ y) * 4;
            TEXTURE(7, x, y) = 128 * (x % 8 && y % 8);
            TEXTURE(8, x, y) = (x - texWidth)*4;

        }
    }
    makepalette(paletteOffset);

    ////generate some textures
    ////  for(int x = 0; x < texWidth; x++)
    ////  for(int y = 0; y < texHeight; y++)
    ////  {
    ////    int xorcolor = (x * 256 / texWidth) ^ (y * 256 / texHeight);
    ////    //int xcolor = x * 256 / texWidth;
    ////    int ycolor = y * 256 / texHeight;
    ////    int xycolor = y * 128 / texHeight + x * 128 / texWidth;
    ////    texture[0][texWidth * y + x] = 65536 * 254 * (x != y && x != texWidth - y); //flat red texture with black cross
    ////    texture[1][texWidth * y + x] = xycolor + 256 * xycolor + 65536 * xycolor; //sloped greyscale
    ////    texture[2][texWidth * y + x] = 256 * xycolor + 65536 * xycolor; //sloped yellow gradient
    ////    texture[3][texWidth * y + x] = xorcolor + 256 * xorcolor + 65536 * xorcolor; //xor greyscale
    ////    texture[4][texWidth * y + x] = 256 * xorcolor; //xor green
    ////    texture[5][texWidth * y + x] = 65536 * 192 * (x % 16 && y % 16); //red bricks
    ////    texture[6][texWidth * y + x] = 65536 * ycolor; //red gradient
    ////    texture[7][texWidth * y + x] = 128 + 256 * 128 + 65536 * 128; //flat grey texture
    ////  }
    //
}

void draw() {
    int x, y, mapX, mapY, stepX, stepY, hit, side, lineHeight, drawStart, drawEnd, texNum, texX, d, texY, cOffset;
    double cameraX, rayPosX, rayPosY, rayDirX, rayDirY, sideDistX, sideDistY, deltaDistX, deltaDistY, perpWallDist, wallX, frameTime, moveSpeed, rotSpeed, oldPlaneX, oldDirX;

    for (x = 0; x < screenWidth; x++) for (y = 0; y < screenHeight; y++) PIXEL(x, y) = 0; //clear the buffer instead of cls()    
    for (x = 0; x < screenWidth; x++) {
        //calculate ray position and direction
        cameraX = 2 * x / (double) screenWidth - 1; //x-coordinate in camera space
        rayPosX = posX;
        rayPosY = posY;
        rayDirX = dirX + planeX*cameraX;
        rayDirY = dirY + planeY*cameraX;

        //which box of the map we're in
        mapX = (int) (rayPosX);
        mapY = (int) (rayPosY);

        //length of ray from current position to next x or y-side
        sideDistX;
        sideDistY;

        //length of ray from one x or y-side to next x or y-side
        deltaDistX = sqrt(1 + (rayDirY * rayDirY) / (rayDirX * rayDirX));
        deltaDistY = sqrt(1 + (rayDirX * rayDirX) / (rayDirY * rayDirY));
        perpWallDist;

        //what direction to step in x or y-direction (either +1 or -1)
        stepX;
        stepY;

        hit = 0; //was there a wall hit?
        side; //was a NS or a EW wall hit?

        //calculate step and initial sideDist
        if (rayDirX < 0) {
            stepX = -1;
            sideDistX = (rayPosX - mapX) * deltaDistX;
        } else {
            stepX = 1;
            sideDistX = (mapX + 1.0 - rayPosX) * deltaDistX;
        }
        if (rayDirY < 0) {
            stepY = -1;
            sideDistY = (rayPosY - mapY) * deltaDistY;
        } else {
            stepY = 1;
            sideDistY = (mapY + 1.0 - rayPosY) * deltaDistY;
        }
        //perform DDA
        while (hit == 0) {
            //jump to next map square, OR in x-direction, OR in y-direction
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            } else {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }
            //Check if ray has hit a wall
            if (worldMap[mapX][mapY] > 0) hit = 1;
        }

        //Calculate distance of perpendicular ray (oblique distance will give fisheye effect!)
        if (side == 0) perpWallDist = (mapX - rayPosX + (1 - stepX) / 2) / rayDirX;
        else perpWallDist = (mapY - rayPosY + (1 - stepY) / 2) / rayDirY;

        //Calculate height of line to draw on screen
        lineHeight = (int) (screenHeight / perpWallDist);

        //calculate lowest and highest pixel to fill in current stripe
        drawStart = -lineHeight / 2 + screenHeight / 2;
        if (drawStart < 0) drawStart = 0;
        drawEnd = lineHeight / 2 + screenHeight / 2;
        if (drawEnd >= screenHeight) drawEnd = screenHeight - 1;

        //texturing calculations
        texNum = worldMap[mapX][mapY] - 1; //1 subtracted from it so that texture 0 can be used!

        //calculate value of wallX
        //where exactly the wall was hit
        if (side == 0) wallX = rayPosY + perpWallDist * rayDirY;
        else wallX = rayPosX + perpWallDist * rayDirX;
        wallX -= floor((wallX));


        //x coordinate on the texture
        texX = wallX * texWidth;
        if (side == 0 && rayDirX > 0) texX = texWidth - texX - 1;
        if (side == 1 && rayDirY < 0) texX = texWidth - texX - 1;
        
        

        for (y = drawStart; y < drawEnd; y++) {
            d = y * 256 - screenHeight * 128 + lineHeight * 128; //256 and 128 factors to avoid floats
            texY = ((d * texHeight) / lineHeight) / 256;

            //Uint32 color = texture[texNum][texHeight * texY + texX];
            uint8_t color = TEXTURE(texNum, texX, texY);
            //            make color darker for y-sides: R, G and B byte each divided through two with a "shift" and an "and"
            //            if (side == 1) color = (color >> 1);
            //flip horizontal, vertical flip done during dispimage
            if (syncMode == LEFTSIDE)
                PIXEL((screenWidth - x), y) = color;
            else 
                PIXEL(x, y) = color;
        }
    }

    //    drawBuffer(buffer[0]);

    //    //timing for input and FPS counter
    //    oldTime = time;
    //    time = getTicks();
    //    frameTime = (time - oldTime) / 1000.0; //frametime is the time this frame has taken, in seconds
    //    print(1.0 / frameTime); //FPS counter
    //    redraw();


    //    drawBuffer(buffer[0]);
    //    for(int x = 0; x < w; x++) for(int y = 0; y < h; y++) buffer[y][x] = 0; //clear the buffer instead of cls()
    //    //timing for input and FPS counter
    //    oldTime = time;
    //    time = getTicks();
    //    double frameTime = (time - oldTime) / 1000.0; //frametime is the time this frame has taken, in seconds
    //    print(1.0 / frameTime); //FPS counter
    //    redraw();
    //
    //    //speed modifiers
    //    double moveSpeed = frameTime * 5.0; //the constant value is in squares/second
    //    double rotSpeed = frameTime * 3.0; //the constant value is in radians/second
    //
}

#define deadzone 500
#define moveScale 32768.0

void readKeys() {

    double moveSpeed = 5.0 / 20; //the constant value is in squares/second
    double rotSpeed = 3.0 / 20; //the constant value is in radians/second
    double oldDirX, oldPlaneX;
    double newX, newY;

    //move forward if no wall in front of you

    if (abs(accz) > deadzone) {
        moveSpeed = 0;
        if (accz > deadzone) {
            moveSpeed = (accz - deadzone) / moveScale;
        }
        if (accz < -deadzone) {
            moveSpeed = (accz + deadzone) / moveScale;
        }

        newX = posX + dirX * moveSpeed;
        newY = posY + dirY * moveSpeed;

        if (worldMap[(int) newX][(int) posY] == 0) posX = newX;
        if (worldMap[(int) posX][(int) newY ] == 0) posY = newY;
    }



    //    if (butpress & but4) {
    //        if (worldMap[(int)(posX + dirX * moveSpeed)][(int)(posY)] == 0) posX += dirX * moveSpeed;
    //        if (worldMap[(int)(posX)][(int)(posY + dirY * moveSpeed)] == 0) posY += dirY * moveSpeed;
    //    }
    //    //move backwards if no wall behind you
    //    if (butpress & but3) {
    //        if (worldMap[(int)(posX - dirX * moveSpeed)][(int)(posY)] == 0) posX -= dirX * moveSpeed;
    //        if (worldMap[(int)(posX)][(int)(posY - dirY * moveSpeed)] == 0) posY -= dirY * moveSpeed;
    //    }


    if (abs(accx) > deadzone) {
        rotSpeed = 0;
        if (accx > deadzone)
            rotSpeed = -(accx - deadzone) / moveScale;
        if (accx < -deadzone)
            rotSpeed = -(accx + deadzone) / moveScale;


        oldDirX = dirX;
        dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
        dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
        oldPlaneX = planeX;
        planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
        planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);

    }
    
//    if (abs(accy) < 16000) {
//        paletteOffset += accy/1000.0;
//        makepalette(paletteOffset);
//    }

    //    //rotate to the right
    //    if (butpress & but2) {
    //        //both camera direction and camera plane must be rotated
    //        oldDirX = dirX;
    //        dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
    //        dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
    //        oldPlaneX = planeX;
    //        planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
    //        planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
    //    }
    //    //rotate to the left
    //    if (butpress & but1) {
    //        //both camera direction and camera plane must be rotated
    //        oldDirX = dirX;
    //        dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
    //        dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
    //        oldPlaneX = planeX;
    //        planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
    //        planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
    //    }
}