// template demonstration application for camera badge
// new apps need to be added to list in applist.h

#include "cambadge.h"
#include "globals.h"

// states used by this application

#define s_start 0
#define s_run 1
#define s_exit 2
#define s_updatemenus 3
#define s_arm 4
#define s_waittrig 5
#define s_stop 6
#define s_forceupdate 7
#define trigtimeout 10 // ticks

#define samples 256 // samples on both channels
#define sampbuf cambuffer_s


static volatile unsigned int ad0, ad1, sampptr, triglevel, timebaseval;
static volatile unsigned char gotsamples, trigstate = 0;

void __ISR(_ADC_VECTOR, IPL6SOFT) adcint(void) {
    unsigned int a, b, c, s0, s1;

    s0 = ADC1BUF0;
    s1 = ADC1BUF1;
    
    switch (trigstate) {
        case 0: if (s0 < triglevel) trigstate = 1;
        
            break; // +edge
        case 1: if (s0 > triglevel) trigstate = 4;
            break;

        case 2: if (s0 > triglevel) trigstate = 3;
            break; // -edge
        case 3: if (s0 < triglevel) trigstate = 4;
            break;

        case 4: PR2 = timebaseval;
        TMR2=0;
            trigstate = 5;
            break;
        case 5:

            sampbuf[sampptr++] = s0;
            sampbuf[sampptr++] = s1;
            if (sampptr >= samples) {
                IEC0CLR = _IEC0_AD1IE_MASK;
                gotsamples = 1;
            }
            break;


    } // switch trigstate


    IFS0CLR = _IFS0_AD1IF_MASK;


}


#define ystart 10
#define yheight 105
#define gridy 13
#define gridx 16
#define xstart 0
#define menus 6

char* scope(unsigned int action) {
    static unsigned int state;
    unsigned int i;

    switch (action) {
        case act_name: return ("BADGESCOPE");
        case act_help: return ("2-channel \nOscilloscope");
        case act_init:
            // add any code here that needs to run once at powerup - e.g. hardware detection/initialisation      
            return (0);

        case act_powerdown:
            // add any code here that needs to run before powerdown
            return (0);

        case act_start:
            // called once when app is selected from menu
            state = s_start;

            return (0);
    } //switch

    if (action != act_poll) return (0);

    static unsigned char trigtimer,runmode, xrange, y1range, y2range, menustate, pb0save, pb1save, trigpol, trigcount, startfudge;
    ;
    static int y1pos, y2pos;
    unsigned int x, y, c, d, ly1, ly2, h, sp;
    int s;
    unsigned char y1base, y2base, trigline;
#define ysf 38912
    const unsigned int yscales[] = {ysf / 8, ysf / 4, ysf / 2, ysf};
#define yranges (sizeof(yscales)/4)
    const char* ynames[] = {shortdot"8", shortdot "4", shortdot "2", shortdot "1"};
    const unsigned int ysteps[yranges] = {16, 8, 4, 2}; // ypos step sizes


#define xsf 15
    const unsigned int xscales[] = {5 * xsf, 10 * xsf, 20 * xsf, 50 * xsf, 100 * xsf, 200 * xsf, 500 * xsf, 1000 * xsf, 2000 * xsf, 5000 * xsf, 10000 * xsf};
#define xranges (sizeof(xscales)/4)
    const char* xnames[] = {"50u", ".1m", ".2m", ".5m", " 1m", " 2m", " 5m", "10m", "20m", "50m", ".1s"};

#define  y1col c_yel 
#define y2col c_grn

#define gridcol rgbto16(128,128,128)
#define y1 yel
#define y2 grn
#define menubg rgbto16(128,128,128)



    switch (state) {
        case s_start:
            printf(cls);
            // drop through
            
           
            claimadc(1);
            ANSELBSET = 3;
            TRISBSET = 3;
            CNPUBCLR = 3;

            pb0save = RPB0Rbits.RPB0R;
            pb1save = RPB1Rbits.RPB1R;
            RPB0Rbits.RPB0R = 0;
            RPB1Rbits.RPB1R = 0;
            menustate = 0;

            T2CON = 0b1000000000001000; // T2 and T3 in 32 bit mode
            T3CON = 0b1000000000000000; // prescale /1
   

            PR2 = 2 * (clockfreq / 2000000) - 1;
            PR2 = 12;
            AD1CON1 = 0b1000000001000100; // trigger on T3
            //                  IIII   int threshold
            AD1CON2 = 0b0000010000000100; //

            AD1CON3 = 0b0000001000000000 | 1; // Tad
            AD1CHS = 2 << 24 | 3 << 16;
            AD1CSSL = 0b0000000000001100; // scan ch2 and ch3
            IPC5bits.AD1IP = 6;

            sampptr = 0;
            y2pos = -512;
            y1pos = -64;
            triglevel = 64;
            y1range = y2range = 0;
            xrange = 0;
            runmode = 1;
            trigpol = 0;
            //startfudge = 1;
            state = s_arm;


        case s_arm:
            IEC0CLR = _IEC0_AD1IE_MASK;
            sampptr = 0;
            gotsamples = 0;

            TMR2 = 0;
            PR2 = 2 * clockfreq / 1000000; // fast sample while awaiting trigger
            timebaseval = xscales[xrange];

            trigstate = trigpol ? 0 : 2;
          
            IFS0CLR = _IFS0_AD1IF_MASK;
            if (runmode) IEC0SET = _IEC0_AD1IE_MASK;
            TMR2 = 4;

            trigtimer=0;
     
            state = s_waittrig;

            break;


        case s_waittrig:
            // we plot level & trig lines at same time as waveform so need to replot waveform if in stop mode

            if(runmode) if(trigstate<4) if(trigtimer>trigtimeout) trigstate=4; // trigger timeout

            if (gotsamples == 0) break;
            // if((runmode==0) && ((tick==0)||(butstate==0))) break; 
        case s_forceupdate:
            sp = 0;
            ly1 = 64;
            ly2 = 64;

            s = -y1pos;
            s = s * (signed int) yscales[y1range] / 65536;
            if (s < 0) s = 0;
            if (s >= yheight - 1) s = yheight - 2;
            y1base = (yheight - s) + ystart - 1; // flip co-ordinates
            s = -y2pos;
            s = s * (signed int) yscales[y2range] / 65536;
            if (s < 0) s = 0;
            if (s >= yheight - 1) s = yheight - 2;
            y2base = (yheight - s) + ystart - 1; // flip co-ordinates

            s = triglevel-y1pos;
            s = s * (signed int) yscales[y1range] / 65536;
            if (s < 0) s = 0;
            if (s >= yheight - 1) s = yheight - 2;
            trigline = (yheight - s) + ystart - 1; // flip co-ordinates


            for (x = xstart; x != 128; x++) {

                plotblock(x, ystart, 1, yheight, ((x - xstart) % gridx) ? 0 : gridcol); //erase column/draw grid vert

                for (y = 0; y != yheight / gridy + 1; y++) plotblock(x, ystart + y * gridy, 1, 1, gridcol); //y lines 
                if ((x - xstart) % (gridx / 2) == 0) plotblock(x, y1base, 1, 1, y1col); // y1 baseline
                if ((x - xstart) % (gridx / 2) == gridx / 4) plotblock(x, y2base, 1, 1, y2col); // y2 baseline
                if ((x - xstart) % (gridx / 2) == 2) plotblock(x, trigline, 1, 1, c_cya); // y2 baseline
                c = y1col;
                s = sampbuf[sp++];
                s -= y1pos;
                s = s * (signed int) yscales[y1range] / 65536;
                if (s < 0) {
                    s = 0;
                    c = c_whi;
                }
                if (s >= yheight - 1) {
                    s = yheight - 2;
                    c = c_red;
                }


                s = (yheight - s) + ystart - 1; // flip co-ordinates
                if (s > ly1) {
                    y = ly1;
                    h = s - ly1;
                } else {
                    y = s;
                    h = ly1 - s;
                } // do vertical bar from last sample Y to this
                if (h == 0) h = 1;
                ly1 = s;
                if (x > xstart) plotblock(x, y, 1, h, c); // simple fudge to avoid glitch moving to first point

                c = y2col;
                s = sampbuf[sp++];
                s -= y2pos;
                s = s * (signed int) yscales[y2range] / 65536;
                if (s < 0) {
                    s = 0;
                    c = c_whi;
                }
                if (s >= yheight - 1) {
                    s = yheight - 2;
                    c = c_red;
                }

                s = (yheight - s) + ystart - 1; // flip co-ordinates
                if (s > ly2) {
                    y = ly2;
                    h = s - ly2;
                } else {
                    y = s;
                    h = ly2 - s;
                } // do vertical bar from last sample Y to this
                if (h == 0) h = 1;
                ly2 = s;
                if (x > xstart) plotblock(x, y, 1, h, c); // simple fudge to avoid glitch moving to first point

            }
            plotblock(127, ystart, 1, yheight, gridcol); //fudge RHS grid line

            printf(top y1);
            if (menustate == 0) bgcol = menubg;
            printf("%s/ " y2, ynames[y1range]);

            if (menustate == 2) bgcol = menubg;
            printf("%s/ " y2, ynames[y2range]);

            printf(whi);
            if (menustate == 4) bgcol = menubg;
            printf("%s", xnames[xrange]);

            printf(cya " ");
            if (menustate == 5) bgcol = menubg;
            i = triglevel * 280 / 1024;
            printf(trigpol ? uarr : darr);
            printf("%d" shortdot"%02dv", i / 100, i % 100);

            printf(tabx17 butcol);
            printf(runmode ? " RUN" : "STOP");
            printf(bot butcol);
            switch (menustate) {
                case 0: printf(y1  "Ch1 V/Div   " uarr "       " darr);
                    break;
                case 1: printf(y1  "Ch1 offset  " uarr "       " darr);
                    break;
                case 2: printf(y2  "Ch2 V/Div   " uarr "       " darr);
                    break;
                case 3: printf(y2  "Ch2 offset  " uarr "       " darr);
                    break;
                case 4: printf(whi "Timebase   mS      uS ");
                    break;
                case 5: printf(cya "Trig Level  " uarr"       " darr);
                    break;

            }

            //printf(tabx0 taby8 "%d %3d %3d %d", trigstate, trigcount++, sampptr, IEC0bits.AD1IE ? 1 : 0);
            state = s_arm;

            break;


        case s_exit:
            IEC0CLR = _IEC0_AD1IE_MASK;
            claimadc(0);
            ANSELBCLR = 3;
            TRISBCLR = 3;
            
            RPB0Rbits.RPB0R = pb0save;
            RPB1Rbits.RPB1R = pb1save;
            CNPUBSET = 1 << 1;
            T2CON = 0; //clear 32 bit mode in case someone uses t3 
            return ("");
            break;


    } // switch state

    if (!tick) return (0);
    trigtimer++;
    if (butpress & powerbut) state = s_exit;



    switch (menustate) {
        case 0: //Y1 v/div
            if (butpress & but2) if (y1range) y1range--;
            if (butpress & but3) if (y1range < yranges - 1) y1range++;
            break;
        case 1: //Y1 pos
            if (butstate & but3) if (y1pos < 2048) y1pos += ysteps[y1range];
            if (butstate & but2) if (y1pos>-1024) y1pos -= ysteps[y1range];

            break;

        case 2: //Y2 v/div
            if (butpress & but2) if (y2range) y2range--;
            if (butpress & but3) if (y2range < yranges - 1) y2range++;
            break;

        case 3: //Y2 pos
            if (butstate & but3) if (y2pos < 2048) y2pos += ysteps[y2range];
            if (butstate & but2) if (y2pos>-1024) y2pos -= ysteps[y2range];

            break;
        case 4: //x range pos
            if (butpress & but3) if (xrange) xrange--;
            if (butpress & but2) if (xrange < xranges - 1) xrange++;

            break;
#define maxtrig 1005
#define mintrig 15
        case 5: //trig level

            if (butstate & but3) {
                if (triglevel < maxtrig - ysteps[y1range]) triglevel += ysteps[y1range];
                else triglevel = maxtrig;
            }
            if (butstate & but2) {
                if (triglevel > mintrig + ysteps[y1range]) triglevel -= ysteps[y1range];
                else triglevel = mintrig;
            }
            if (butpress & but5) {
                trigpol ^= 1;
            }

            break;


    } // menustate

    if (butpress & but1) if (++menustate == menus) menustate = 0;
    if (butpress & but4) runmode ^= 1;
    if(butpress & but5) if(runmode) trigstate=4;
    if (butstate & butmask) state = s_forceupdate;
    
    
    return (0);

}


