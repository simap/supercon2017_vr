// Camera badge main root source
// currently appears to be an issue with optimization level >0 throwing an exception. 
// appears fixed with compiler flag -fno-aggressive-loop-optimizations

#include "cambadge.h"
#include "globals.h"

#if debug_dma==1
#warning debug_dma enabled - SD card not useable
#endif

#pragma config USERID=0x4248 //"HB" - used by bootloader to validate correct image for the hardware. 
//NB bytes reversed in #pragma config vs. bootloader values

//config data -  only used when programming standalone - bootloader config is used for normal operation
#if clockfreq==48000000
#pragma config FPLLMUL = MUL_24, FPLLIDIV = DIV_2, FPLLODIV = DIV_4
#define ckf 1
#endif
#if clockfreq==40000000
#pragma config FPLLMUL = MUL_20, FPLLIDIV = DIV_2, FPLLODIV = DIV_2
#define ckf 1
#endif

#pragma config FWDTEN = ON,WDTPS = PS2048,WINDIS=OFF  // PS2048 = approx 2 sec WDT
#pragma config  FNOSC = FRCPLL, FPBDIV = DIV_1,FSOSCEN=OFF,FCKSM=CSECME
#pragma config JTAGEN = OFF,OSCIOFNC=OFF
#pragma config IOL1WAY=OFF,PMDL1WAY=OFF


//================================================================================================
// UI states 

#define s_startup 0
#define s_waitsplash 1
#define s_splashavi 2
#define s_showstartup 3
#define s_sswait 4
#define s_restart 5

#define s_powerdown 6
#define s_powerdownwait 7

#define s_appstart 8
#define s_apprun 9
#define s_appquit 10

#define s_startmenu 11
#define s_showmenu 12
#define s_waitmenu 13
#define s_startapp 14

#define s_restartmenu 15


#include "appmap.h"
typedef char* (*application)(unsigned int);
application const apps[] = {applist}; // list of function pointers to applications

void main(void) {
    unsigned int i, state, dispowerdown = 0;
    unsigned int powerbuttimer = 0, mounttimer = 0; // power button-down timer, auto poweroff timer, card detect debounce timer
    unsigned int y, scroll, appnum = 0; // menu display

    
    
    inithardware();
    setupints();
    readbuttons();

    if (butstate & but4) selftest();
    acc_init();

    state = s_startup;
    cardmounted = 0;
    IFS0SET = _IFS0_T4IF_MASK; // force initial timer tick for button read, battery level initialise 
    TMR4 = 0;

    printf(cls blu "Starting" whi); // in case something crashes early

    // check for i2c jam issue from camera - shouldn't happen nowadays but useful as a quick diagnostic 
    I2C2CONbits.ON = 0;
    Nop();
    Nop();
    i = PORTB & 0x0c;
    I2C2CONbits.ON = 1;
    if (i != 0x0c) printf(" I2C Jammed" del);

    for (i = 0; i != napps; apps[i++](act_init)); // let apps do hardware initialisation
    powerdowntimer = 0;
    do { // main poll loop
        kickwatchdog; // reset watchdog timer
        tick = 0; // event flags to be set in tick but only for one pass through main loop
        butpress = 0;
        cardinsert = 0;

        if (IFS0bits.T4IF) { //  timer tick. nominally 20mS but can be longer of tasks take more time
            tick = 1 + TMR4 / (ticktime * (clockfreq / 1000000) / t4prescale); // estimate number of missed ticks
            TMR4 = -(ticktime * (clockfreq / 1000000) / t4prescale); // reset timer
            LATCINV = 1 << 5;
            IFS0CLR = _IFS0_T4IF_MASK;
            readbatt(); // read battery voltage
            readbuttons();
        
            i = accx; // previous accx for change detect
            acc_read();
            if (abs((signed int) i - accx) > accmovethresh) powerdowntimer = 0; // check for movement
            if (butpress) powerdowntimer = 0;
            if (powerdowntimer > powerdowntime) if (state != s_powerdownwait) state = s_powerdown;
            if (!dispowerdown) powerdowntimer += tick;
            if (butstate & powerbut) powerbuttimer += tick; // power button hold timer
            else powerbuttimer = 0;
            if (powerbuttimer > powerbuttime) if (state != s_powerdownwait) state = s_powerdown;
#if debug_dma==0   
            sd_cs_in; // use delay through readbuttons for SD Card-detect pulldown setting time
            delayus(5); // wait settling time
            if (!sd_carddet) {
                if (cardmounted) cardinsert = 1; // card-remove event
                mounttimer = 0;
                cardmounted = 0;
            } else ++mounttimer; // will overflow eventually but not for about 1.3 years so ignore it for simplicity
            sd_cs_out; // return SD CS to output
            if (mounttimer == cardmounttime) {
                sd_cs = 0;
                ;
                delayus(100);
                sd_cs = 1;
                if (FSInit()) {
                    cardmounted = 1;
                    cardinsert = 1;
                } else mounttimer = 0; // avoid hang waiting for bad card to mount
            }
#endif   

        }// tick

#if serialcontrol==1
        polluart();
#endif

        switch (state) {
            case s_startup:
                if (butstate) {
                    state = s_showstartup; // show startup options
                    break;
                }
                if (mounttimer == 0) state = s_restart; // card not present at startup, or card error
                if (cardmounted) {
                    state = s_restart;
                    if (!openavi("splash.avi")) { // splash vid present
                        state = s_splashavi;
                        break;
                    }
                    if (!loadbmp("splash.bmp", 2)) {
                        state = s_waitsplash;
                        break;
                    }
                }
                break;

            case s_showstartup: // show startup button options
                printf(cls butcol "           Selftest "uarr tabx0 taby9 tabx13 "Disable\n" tabx13 "Auto\nBootloader   Poweroff\n  "darr tabx19 " " darr);
                printf(tabx0 taby2 whi inv "POWER-UP OPTIONS:"inv"\n\nHold button when\npowering on");
                if (butstate & but3) {
                    dispowerdown = 1;
                    printf(tabx0 taby7 inv yel "Powerdown disabled");
                }
                state = s_sswait;
                break;

            case s_sswait:
                powerbuttimer = 0;
                if (!butstate) state = s_startup;
                break;


            case s_waitsplash: // hold bmp splashscreen
                if (mounttimer > cardmounttime + splashtime) state = s_restart;
                break;

            case s_splashavi: // play avi splashscreen
                i = showavi(); // show a frame
                if ((i != 0) || (avi_framenum == avi_frames - 1)) {
                    FSfclose(fptr);
                    state = s_restart;
                }
                break;

            case s_restart:

                state = s_startmenu;
                break;

            case s_startmenu:
                appnum = 0;
                scroll = 0;

            case s_restartmenu:
  #if acceltype==accel_dh
                printf(cls whi "Proto accel version" del);
#endif
                led1_off;
                dispuart = 0;

                printf(whi cls version bot butcol "  " uarr "         " darr "      Go");

#if debug_dma==1
                printf(top red inv "DEBUG_DMA ON" inv whi);
#endif

#define maxy 7   // number of apps in menu  
            case s_showmenu:
                if (napps > maxy && scroll + maxy < napps) printf(tabx18 taby8 whi "...");
                else printf(tabx18 taby8 whi "   ");
                printf(whi tabx0 taby2);
                dispy -= 3; // vertically centre
                y = 0;

                do {
                    if (y + scroll == appnum) {
                        bgcol = fgcol; //invert
                        fgcol = c_blk;
                    } // currently selected
                    printf("%-20s\n" whi, apps[y + scroll](act_name));
                    y++;
                } while (y < napps && (y < maxy));

                plotblock(0, dispy, dispwidth, dispheight - dispy - charheight, 0); // clear help text area ( not using cls to avoid flicker of names and button legends)
                printf(taby9 yel "%s", apps[appnum](act_help)); // help text
              //  printf(taby7 tabx0  cya  "Please Update\nfirmware from card" );
                state = s_waitmenu;
                break;

            case s_waitmenu: // wait for button press
                if (!tick) break;
                // following prints done every tick - not efficient but we've nothing else to do
                printf(tabx12 taby0 whi);
                printf(cardmounted ? sdcd : " "); // SD card symbol
                // display battery voltage
                printf(battlevel > batthresh1 ? grn : battlevel < batthresh2 ? red : yel); //select colour
                printf(tabx13 taby0 bat "%c%d.%02dV", dispowerdown ? 0x60 : ' ', battlevel / 1000, (battlevel % 1000) / 10);

                if (!butpress) break;

                state = s_showmenu; //default next state
                if (butpress & but1) { // up 
                    if (appnum) appnum--;
                    if (appnum < scroll) scroll--;
                }

                if (butpress & but2) { // down
                    if (appnum < napps - 1) appnum++;
                    if (appnum - scroll >= maxy) scroll++;
                }

                if (butpress & (but3 | but5)) state = s_appstart;
                break;

            case s_appstart:
                printf(whi cls);
                apps[appnum](act_start); // start application
                state = s_apprun;
                break;

            case s_apprun:
                if (apps[appnum](act_poll)) state = s_restartmenu; // poll application
                break;

            case s_powerdown:
                printf(cls);
                if (butstate & powerbut) break; // wait til button release
                // will not normally be seen, but will if power override link is fitted
                printf(cls taby4 grey "Awaiting Powerdown\n\n Button 5:reset");
                cam_enable(cammode_off);
                
                
                led1_off;
                for (i = 0; i != napps; apps[i++](act_powerdown)); // let apps do hardware de-init
                state = s_powerdownwait;
                break;

            case s_powerdownwait:
                /// issue found on some badges is that when PIC goes into reset, pin leakage from RA4 turns supply backon
                // ideal fix would be to reduce R8, but need to do it in software...
                // method is to PWM power control to reduce supply & discharge capacitance for a while before finally turning off, so 
                // insufficient energy left to power-up again
                delayus(200000);// wait till effect of button via D28 is gone
                 __builtin_disable_interrupts();

                for(i=0;i!=1000;i++){
                  powercon_on;
                  delayus(100);
                  powercon_off;
                  delayus(100);
                  kickwatchdog;
                }
                powercon_off;
           
                do { // only gets here if power-bypass link set
                    kickwatchdog;
          
                   if(PORTAbits.RA9)__pic32_software_reset();
                } 
                    while (1);

                break;

            default: state = s_startup;

        } // switch state


    } while (1);

}




