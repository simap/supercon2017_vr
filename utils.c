
#include "cambadge.h"
#include "globals.h"

char* settings(unsigned int action) {
    unsigned int i, j, t;
    static unsigned char tport, tbyte;
    static unsigned int u1rxcount;
    static unsigned char state = 0;
#define s_sstart 0
#define s_idle 1
#define s_formwait 2
#define s_formwait2 3
#define s_twidstart 4
#define s_twiddle 5
#define s_speedtest 6
#define s_stwait 7


    if (action == act_name) return ("UTILITIES");
    else if (action == act_help) return ("Various oddments");

    if (action == act_start) state = s_sstart;

    if (action != act_poll) return (0);
    if (butpress & powerbut) return ("");

    switch (state) {

        case s_sstart:
            printf(cls butcol "EXIT  " whi inv "UTILITIES" inv butcol "  Boot" bot "Utils   twiddler" tabx0 taby11 "Card    Hardware");

            state = s_idle;
            break;


        case s_idle:
            if (!tick) break;

            printf(tabx0 whi taby2 "X: %6d\nY: %6d\nZ:%6d\n\n", accx, accy, accz);

            if (butpress & but1) {
                printf(cls top "EXIT" bot "Speedtest      Format");
                state = s_formwait;
                break;
            }

            if (butpress & but2) state = s_twidstart;


            if (butpress & but4) {
                do {
                    kickwatchdog;
                    delayus(10000);
                    readbuttons();
                } while (butstate);
                reboot();
                break;
            }

            break;
        case s_formwait:

            if (!butpress) break;
            printf(cls top "EXIT" tabx14 "Really" taby1 tabx13"Confirm");
            if (butpress & but1) {
                state = s_speedtest;
                break;
            }
            if (butpress & but3) state = s_formwait2;
            else state = s_sstart;
            break;

        case s_speedtest:
            
            if (cardmounted == 0) {
                state = s_sstart;
                break;
            }
            state = s_stwait;
            
            printf(cls whi "Opening" tabx0);
            T5CON = 0b1000000001110000; // timer to measure grab+save time for playback framerate
            PR5 = 0xffff;
            TMR5 = 0; // timer on, /256 prescale
            fptr = FSfopen("Speedtst.dat", FS_WRITE);
            if(fptr==NULL) {printf("CARD ERROR");break;}
   
            u1rxcount = 0;
#define passes 7
#define wsize (128*96*2+8)
            for (i = 0; i != passes; i++) {
                TMR5 = 0;
                IFS0bits.T5IF = 0;
                j = FSfwrite(&cambuffer[0], wsize, 1, fptr);
                if(j==0) printf("ERR");
                t = TMR5;
                if (IFS0bits.T5IF) t += 0x10000; // rolled - assume only once
                t = (t * 256 / (clockfreq / 1000)); // mS
                u1rxcount += t;
             
                    printf("Blk %1d T=%5dmS", i, t);
                    if (IFS0bits.T5IF) printf("?"); //possible overflow
                    printf("\n");

            }//for

            FSfclose(fptr);
             printf("\n%4d Kbytes/Sec\n~%2d FPS (RGB)", wsize * passes * 1024 / 1000 / u1rxcount, 1000 * passes / u1rxcount);

            FSremove("Speedtst.dat");
            

            break;
        case s_stwait:
            if (!tick) break;
            printf(tabx0 taby11 whi "Card status: ");
            if (cardmounted) printf("OK  " bot butcol "Repeat          Exit");
            else printf("None " bot butcol "                Exit");

            if (butpress & but3) state = s_sstart;
            if (butpress & but1) if (cardmounted)state = s_speedtest;
            break;



        case s_formwait2:
            if (!(butpress & but4)) break;

            sd_cs_in;
            delayus(1000);
            i = sd_carddet;
            sd_cs_out;
            state = s_sstart;
            if (!i) {
                printf(cls "No card" del del);
                break;
            }
            if (butpress & but4) {
                printf(cls top "\n\nFormatting ");
                if (FSformat(0, 1234, "HADBADGE")) printf("\nFailed" del del);
                else printf("\nFormatted OK" del del);


            }
            break;

        case s_twidstart:
            printf(cls butcol "EXIT              Tx1" bot "OutPort   Flip    I2C");
            state = s_twiddle;
            tport = 0;
            tbyte = 0x55;
            break;

        case s_twiddle:
            if (U1MODEbits.ON) if (U1STAbits.URXDA) printf(tabx8 taby6 "U1RX %02X #%04d", U1RXREG, ++u1rxcount);
            if (!tick) return (0);


            printf(tabx0 taby1 whi "    A2 B0 B1 B4 C3 C5" tabx0 taby2 "In" tabx0 taby3 "Out");


            printf(tabx4 taby2 "%1d  %1d  %1d  %1d  %1d  %1d", PORTAbits.RA2 ? 1 : 0, PORTBbits.RB0 ? 1 : 0, PORTBbits.RB1 ? 1 : 0, PORTBbits.RB4 ? 1 : 0, PORTCbits.RC3 ? 1 : 0, PORTCbits.RC5 ? 1 : 0);
            printf(tabx4 taby3 "%1d  %1d  %1d  %1d  %1d  %1d", LATAbits.LATA2 ? 1 : 0, LATBbits.LATB0 ? 1 : 0, LATBbits.LATB1 ? 1 : 0, LATBbits.LATB4 ? 1 : 0, LATCbits.LATC3 ? 1 : 0, PMADDR & 8 ? 1 : 0);
            for (i = 0; i != 6; i++) printf(taby4"%c%c", 0x84 + i * 3, (i == tport) ? 0x18 : ' ');

            if (butpress & but1) if (++tport == 6) tport = 0;

            if (butpress & but4) {

            }

            if (butpress & but2)
                switch (tport) {
                    case 0: LATAINV = 1 << 2;
                        break;
                    case 1: LATBINV = 1 << 0;
                        break;
                    case 2: LATBINV = 1 << 1;
                        break;
                    case 3: LATBINV = 1 << 4;
                        break;
                    case 4: LATCINV = 1 << 3;
                        break;
                    case 5: //LATCINV = 1 << 5;
                        PMADDR = (PMADDR & 8) ? PMADDR&~8 : PMADDR | 8;
                        break;
                }


            if (butpress & but3) {

                printf(top taby9 whi "I2C:                    " taby9 tabx4);

                I2C2BRG = clockfreq / 2 / i2cspeed_cam;
                for (i = 0x10; i != 0xfe; i += 2) {
                    j = iistart(i);
                    iistop();
                    if (j) printf("%02X ", i);
                }

            }



            if (butpress & but4) {
                printf(tabx0 taby6 "U1TX %02X", tbyte);
                u1txbyte(tbyte++);
            }

            if (U1MODEbits.ON) printf(tabx0 taby7 "U1 Baud=%d", u1baud);

    }//switch



    return (0);
}