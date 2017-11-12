
#include "cambadge.h"
#include "globals.h"

// serial control - mostly a remnant from the old serial camera board, some parts may not entirely work.
// useful for messing with camera & OLED commands etc. as well as getting live image data
// for image data you will probably want to crank up the baudrate to 2Mbaud or higher


#if serialcontrol==1

unsigned int camoptions;

void polluart(void) { // check for serial commands. Slightly crude but simple

    unsigned int i, pchange, x, y, outptr;
    unsigned char c;

    if (tick) {
        if (rxptr) if (++rxtimer > 2) rxptr = 0; // serial receive timeout

        if (U2STAbits.OERR) {
            U2STAbits.OERR = 0;
            rxptr = 0;
        } // UART overrun
        if (U2STAbits.FERR) {
            U2STAbits.FERR = 0;
            rxptr = 0;
        } // Framing error
    }


    if (rxptr == 0) return;

    switch (rxbuf[0]) {


        case 0x10:// 0x10 <adr> <value>  set camera register. returns 0x55 

            if (rxptr != 3) break;
rxptr = 0;
            cam_setreg(rxbuf[1], rxbuf[2]);   
            u2txbyte(0x55);
            break;

        case 0x11: // 0x11 <rr> read camera register. returns 1 byte register value
            if (rxptr != 2) break;
            rxptr = 0;
            u2txbyte(readcamreg(rxbuf[1]));
            break;

         case 0x13: // 0x12 <en> <graben> camera enable/disable
            if (rxptr != 3) break;
                rxptr=0;
            cam_enable(rxbuf[1]);
            cam_grabenable(rxbuf[2],0,0);
            u2txbyte(0x55);
            break;   
            

        case 0x14: // 14 xstart/2 ystart/2 xdiv ydiv refclkdiv  set capture parameters 0 = unchanged
            
            if (rxptr != 6) break;
            rxptr = 0;
            if (rxbuf[1]) xstart = (unsigned int) rxbuf[1]*2;
            if (rxbuf[2])ystart = (unsigned int) rxbuf[2]*2;
            if (rxbuf[3])xdiv = (unsigned int) rxbuf[3];
            if (rxbuf[4])ydiv = (unsigned int) rxbuf[4];
            if (rxbuf[5]) REFOCON = 0b1001001000000000 | ((unsigned int) rxbuf[5]) << 16; // XCLK clock to cam
            u2txbyte(0x55);
            
            break;
            
        case 0x15 : // 15  poll camera state
            if(rxptr!=1) break;
            rxptr=0;
            u2txbyte(cam_newframe?1:0 | cam_busy?2:0 | cam_wrap?4:0 | cam_started?8:0);
            u2txword(linecnt);
            u2txword(camaddr);
            break;

        case 0x16 : // 16 xx return image. xx=1 to wait for new frame
              // could be optimised by starting to send while capture in progress
             if(rxptr!=2) break;           
             rxptr=0;
             if(rxbuf[1]) while(!cam_newframe);
             for(x=1,i=0;i!=xpixels*ypixels*(camflags & camopt_mono)?1:2;i++) {
                 kickwatchdog;
                 u2txbyte(cambuffer[x++]);
             }
             cam_newframe=0;
             break;
             

        case 0x20: // return  info
            if(rxptr!=1) break;
            rxptr=0;
            u2txbyte(versionbyte);

            u2txbyte(clockfreq / 1000000);
            u2txbyte(cambufsize >> 16);
            u2txbyte(cambufsize >> 8);
            u2txbyte(cambufsize & 0xff);


            rxptr = 0;
            break;

        case 0x30: // 30 return accel data, bat level, bat state
            if(rxptr!=1) break;
            rxptr=0;
            u2txword(accx); 
            u2txword(accy);
            u2txword(accz);
            u2txbyte(butstate); 
            u2txbyte(butpress);
            u2txword(battlevel);
            break;            

            
            
            
    }//switch
    if (rxptr == 0) powerdowntimer = 0; // reset timer on good command
}
#endif

