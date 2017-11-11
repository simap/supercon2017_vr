
#include "cambadge.h"
#include "globals.h"

// hardware drivers, general purpose routines.
// see globals.h for documentation of useful stuff

void _general_exception_handler(unsigned cause, unsigned status) {
    // catch this to avoid confusion over cause of fatal crashes like div by zero or memory over-pissage 
    // default handler just resets
    // don't use printf in case it's a stack issue 
    dispuart=0;
    dispchar(0xe7); // white on blue
    dispchar('B');
    dispchar('S');
    dispchar('O');
    dispchar('D');
    while (1); // watchdog timeout    
}


//


//================================================================= delay

void do_delay(unsigned int d) // delay in clocks, used by delay_us macro
{
    unsigned long dd;
    T1CON = 0b1000000000000000; // timer on, no prescale
    do {
        if (d > 0xffff) dd = 0xffff;
        else dd = d; // may exceed counter range
        PR1 = dd;
        TMR1 = 0;
        IFS0CLR = _IFS0_T1IF_MASK;
        ;
        while (!IFS0bits.T1IF) kickwatchdog;
        d -= dd;
    } while (d);
}

//====================================================== I2C

void iistop(void) {
    I2C2CONbits.PEN = 1;
    while (I2C2CONbits.PEN);
}

unsigned int ACKStatus(void) {
    return (!I2C2STATbits.ACKSTAT);
}

unsigned int getiic(unsigned int ack) {
    unsigned int t = 10000;
    I2C2CONbits.RCEN = 1; //Enable Master receive
    Nop();
    while ((!I2C2STATbits.RBF) && (--t)); //Wait for receive buffer to be full
    if (t == 0) {
        I2C2CONbits.RCEN = 0;
        return (0);
    } // timeout
    I2C2CONbits.ACKDT = ack ? 0 : 1;
    I2C2CONbits.ACKEN = 1;
    while (I2C2CONbits.ACKEN);
    return (I2C2RCV);
}

unsigned int iisend(unsigned char byte) {
    while (I2C2STATbits.TRSTAT); //Wait for bus to be idle
    I2C2TRN = byte; //Load byte to I2C1 Transmit buffer
    while (I2C2STATbits.TBF); //wait for data transmission
    while (I2C2STATbits.TRSTAT); // wait bus idle
    return (!I2C2STATbits.ACKSTAT);
}

void iirestart(unsigned char iiadr) {
    I2C2CONbits.RSEN = 1;
    while (I2C2CONbits.RSEN);
    iisend(iiadr);
}

unsigned int iistart(unsigned char iiadr) {

    I2C2CONbits.SEN = 1;
    while (I2C2CONbits.SEN);
    return (iisend(iiadr));

}

void cam_setreg(unsigned char r, unsigned char d)
//set camera register
{
    I2C2BRG = clockfreq / 2 / i2cspeed_cam;
    iistart(iicadr_cam);
    iisend(r);
    iisend(d);
    iistop();
}

void acc_setreg(unsigned char r, unsigned char d)
// set accelerometer register
{
    iistart(iicadr_acc);
    iisend(r);
    iisend(d);
    iistop();
}

unsigned char readcamreg(unsigned char c) {
    I2C2BRG = clockfreq / 2 / i2cspeed_cam;
    char d;
    iistart(iicadr_cam);
    iisend(c);
    iistop();
    iistart(iicadr_cam | 1);
    d = getiic(0);
    iistop();
    return (d);
}

void acc_init(void) {

    I2C2BRG = clockfreq / 2 / i2cspeed_norm;
    // set up accelerometer

    //iistop(); // reset any I2C devices that may be part-way through an aborted transaction
    iistart(iicadr_acc);
    #if acceltype==accel_dh 

    iisend(0x9f); // reg 1f+auto-increment
    iisend(0xC0); // temp control
    iisend(0x47); // ctrl reg1 4 msbits = sample rate 1..7 = 1/10/25/50/100/200/400Hz
    iisend(0x00); // ctrl_reg2
    iisend(0x00); // ctrl_reg3
    iisend(0x88); // ctrl_reg4 // b7=block update, b3 = high res
    iisend(0x00); // ctrl_reg5

#endif
#if acceltype==accel_hh 
   
    
    iisend(0x20); // ctrl reg 1
    iisend(0xdF); // ctrl reg 1 hires, 50Hz BDU XYZ en

    
#endif    
    
    iistop();
}

void acc_read(void) { // read all accelerometer values
    unsigned int status, i;

    I2C2BRG = clockfreq / 2 / i2cspeed_norm;
    iistart(iicadr_acc);
#if acceltype==accel_dh
    iisend(0xa7); // b7 set for auto-increment
#else
    iisend(0x27);
#endif    
    
    iirestart(iicadr_acc + 1);
    status = getiic(1); // ignore for the moment
    accx = getiic(1);
    accx |= (getiic(1) << 8); // swap x/y due to board orientation
    accy = getiic(1);
    accy |= (getiic(1) << 8);
    accz = getiic(1);
    accz |= (getiic(0) << 8);

    if (accx & 0x8000) accx |= 0xffff0000; // sign-extend
    if (accy & 0x8000) accy |= 0xffff0000; // sign-extend
    if (accz & 0x8000) accz |= 0xffff0000; // sign-extend
    accx = -accx; // correct for orientation

    iistop();
}

//=========================================================================== cam control

void cam_grabdisable(void) {
   
    cam_newframe = 0;
    cam_started = 0;
    IEC0CLR = _IEC0_INT4IE_MASK; // disable vsync int
    while (cam_busy); // wait for any in-progress grab to finish  

}

void cam_grabenable(unsigned int opt, unsigned int bufoffset, unsigned int cambuflen) {// enable acquisition from running camera
#define camtimeout 50 // mS
    unsigned int i;
    cam_grabdisable();
    cam_stop = (opt == camen_grab) ? 1 : 0;
    camoffset = bufoffset;
    cammax = cambuflen;
    i=xpixels*((camflags&camopt_mono)?1:2);
    if ((cammax == 0) || (cammax > cambufsize - i)) cammax = cambufsize - i; // add margin so we don't need to worry about line length 

    IFS0CLR = _IFS0_INT4IF_MASK;
    IEC0SET = _IEC0_INT4IE_MASK; //enable vsync int

}

void cam_enable(unsigned int mode) {//initialise & enable camera, Data will be at bufoffset+1 due to PMP buffering
    static unsigned int lastmode = 0;
    unsigned int i, j;
#include "camvals_9650.h"

    cam_grabdisable(); // stop any current grabbing
    REFOCON = 0b1001001000000000 | refclkdiv << 16; // fast while setting up regs - underclocking messes up I2C, causing bus jam
    T3CON = 0b1000000000000010; //external clock. PR values get set up in vertical sync int
    
    cammode = mode; // global current-mode value
    if (cammode == 0) {//disable
        cam_setreg(0x09, 0x10); // soft standby
        // NB a second or so after doing hard powerdown, camera jams I2C SDA for a second or so as Vcore discharges,
        // causing I2C peripheral to hang. Only use hardware powerdown for soft 'off' mode, not routine cam enable/disable.
        //  oledcmd(0x1b5);oledcmd(0x0f); // cam hard powerdown ( uses OLED GPIO))
        lastmode = 0; // ensure reinitialised next time
        return;
    }

    // enable 

    if (cammode >= ncammodes) cammode = 1;
    i = camconfig[lastmode].flags;
    camflags = camconfig[cammode].flags;
    if ((i^camflags) & camopt_mono) lastmode = 0; // force full initialise if changing colour/mono


    xpixels = camconfig[cammode].xpixels;
    ypixels = camconfig[cammode].ypixels;
    xdiv = camconfig[cammode].xdiv;
    ydiv = camconfig[cammode].ydiv;
    xstart = camconfig[cammode].xstart;
    ystart = camconfig[cammode].ystart;

    if (lastmode == 0) { // only fully initialise if previously disabled, or changing mono/colour,for speed - cam takes a sec or two to stabilise after full reset
        lastmode = mode;
        // pulse camera powerdown to reset in case it's jamming the i2C bus - it sometimes does this if underclocked
        oledcmd(0x1b5);
        oledcmd(0x0f);
        delayus(100);
        oledcmd(0x1b5);
        oledcmd(0x0C);
        delayus(10000);
        for (i = 0; i != sizeof (camvals_9650) / 2; i++) // /2 as array is 16 bit shorts 
        {
            j = camvals_9650[i];
            cam_setreg(j >> 8, j);
            if (i == 0) delayus(500); // initialise takes a while
        }
    }
    i = (camflags & camopt_vga) ? 0x40 : 0x10;
    if (!(camflags & camopt_mono)) i |= 4;
    cam_setreg(0x12, i); // VGA mode
    cam_setreg(0x3a, (camflags & camopt_swap) ? 0x08 : 0x00); // swap bytes
    cam_setreg(0x15, (camflags & camopt_clkphase) ? 0x22 : 0x32); // swap pixclk phase
    if (camflags & camopt_mono) { // mono mode 
        for (i = 0; i != sizeof (camvals_9650_mono) / 2; i++) {
            j = camvals_9650_mono[i];
            cam_setreg(j >> 8, j);
        }
    }//mono


    REFOCON = 0b1001001000000000 | ((((camflags >> 4)&7) + 2) << 16); // final clock rate after i2C done

}




//============================================================================ UART 


void u1txbyte(unsigned int c) {//Send byte to UART 1. Does not wait for completion
    if(U1MODEbits.ON==0) {
  
    //UART1  - on expansion header
            // fedcba9876543210
    U1MODE = 0b1000000000001000; // BRGH=1
    U1STA =  0b0001010000000000; // txen,rxen
    U1BRG = clockfreq / u1baud / 4 - 1;
    iosetup_uart1;
    }
    
    
    while (U1STAbits.UTXBF);
    U1TXREG = c;
}


void u2txbyte(unsigned int c) { // send byte to UART 2
    while (U2STAbits.UTXBF);
    U2TXREG = c;
}

void u2txword(unsigned int c) {
    u2txbyte(c);
    u2txbyte(c >> 8);
}

unsigned int sendspi(unsigned int d) {
    SPI2BUF = d;
    while (SPI2STATbits.SPIRBF == 0);
    return (SPI2BUF);

}

int randnum(int min, int max) { // return signed random number between ranges
    return (rand() % (max - min) + min);
}
//========================================================================= inithardware
void claimadc(unsigned char claim)
{
 
    if(claim) adcclaimed=1; 
    else {
    IEC0CLR = _IEC0_AD1IE_MASK;
    
            // ADC  - battery measure
    // fedcba9876543210
    AD1CON1=0;
    AD1CON1 = 0b1000000011100000; //auto convert after sampling time, FRC clock
    AD1CON2 = 0;
    //           sssss
    AD1CON3 = 0b1000100000000000; // sample time.
    AD1CSSL=0;
    adcclaimed=0;
    
    }
    
}




void inithardware(void) {// note hardware may not be in reset state as some stuff gets initialised by bootloader
    unsigned int i;

    powercon_on;
    ///////// fedcba9876543210
    TRISA = 0b1111101011100000;
    TRISB = 0b0111111111101110;
    TRISC = 0b1111111110011001;
    ANSELA = ANSELB = ANSELC = 0;
    LATA = LATB = LATC = 0;
    powercon_on;
    CNPUA = CNPUB = CNPUC = CNPDA = CNPDB = CNPDC = 0;
    CNPUCbits.CNPUC3=1; 
    LATCbits.LATC3=1; // u1 tx idle state for when UART enabled by u1tx

    // switch to final clock frequency. 48MHz is technically overclocking if part is 40Mhz so bootloader sets 40MHz in config

#if clockfreq==48000000
#define pllmult 7 // Pll mul 0-7 = 15..21,24
#define pllodiv 1 // 0-3 = 1,2,4,8,
#define ckf 1
#endif

#if clockfreq==40000000
#define pllmult 5 // Pll mul 0-7 = 15..21,24
#define pllodiv 1 // 0-3 = 1,2,4,8,
#define ckf 1
#endif
#ifndef ckf
#error no clockdiv
#endif

    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA; //unlock sequence to write osc
    //       fedcba9876543210fedcba9876543210
    OSCCON = 0b00000001000000000001000100000001 | (pllodiv << 27) | (pllmult << 16);

    while (!OSCCONbits.SLOCK);
    REFOCON = 0b1001001000000000 | refclkdiv << 16; // fast while setting up cam regs - underclocking cam messes up I2C, causing bus jam

    iosetup_leds;
    iosetup_butts;
    iosetup_readbat;
    iosetup_powerbut;
    butts_out;

    led1_off;
    claimadc(0);
    


    // Timer 1 - used for do_delay/delayus() for general purpose delays
    T1CON = 0b1000000000000000; // timer on, no prescale

    // T3 pixel clock divider to trigger DMA transfer of camera data
    T3CON = 0b1000000000000010; //external clock. PR values get set up in vertical sync int
    iosetup_cam;
    
    //T2 frame interval timer used by browser and camera
    T2CON = 0b1000000001110000; // prescale 256
    // T4 system tick timer
    T4CON = 0b1000000000000000 | (t4prescalebits << 4);
    PR4 = 0xffff; // allow rollover so if a task takes a long time we can still keep track of real time for button times etc.


#if (ticktime*(clockfreq/1000000)/t4prescale)>65535
#error tick timer T4 PR value out of range - change prescaler
#endif

    // parallel port for camera data
    // fedcba9876543210
    PMCON = 0b1000000000000000; // B8 enable read strobe
    PMMODE = 0b0000001000000000;
    PMAEN = 1<<3; // note due to silicon errata, IO output on pins shared with PMADDR doesn't work, 
    PMADDR=1<<3; // default state of U1 TXD
    // so need to use PMADDR to access and enable the corresponding bit in PMAEN 
    REFOCON = 0b1001001000000000 | refclkdiv << 16; // XCLK clock to cam  to avoid I2C jam

#if debug_dma==1 // enable RD to scope DMA read requests on pin
    PMCONbits.PTRDEN = 1;
    TRISBbits.TRISB13 = 0;
#endif

 // uart1 only initialised on first use of u1txbyte so can be used as IO

    //uart 2 tx/rx debug (via ISP header ) 
    // fedcba9876543210
    U2MODE = 0b1000000000001000; // BRGH
    U2STA = 0b0001010000000000; // TXEN,RXEN. Int on first byte to avoid FIFO timeout issue -timers are heavily used so don't want to waste one for UART fifo timeout
    U2BRG = clockfreq / u2baud / 4 - 1;
    iosetup_uart2;

    //SPI1 OLED display

    SPI1CON = 0; // may have been previously enabled by bootloader
    // fedcba9876543210
    SPI1CON = 0b10000000100110000; // master, no Din, 8 bit
    SPI1CON2 = 0;
    SPI1STAT = 0;
    SPI1BRG = (clockfreq / 2) / oledclk - 1;
#if((clockfreq/2)/oledclk) * oledclk!=clockfreq/2
#warning invalid oledclk divider value
#endif
    SPI1CONbits.ON = 1;
    iosetup_oled;

    //SPI2 SD card
    sd_cs = 1;
    SPI2CON = 0; // may have been previously enabled by bootloader
    // fedcba9876543210
    SPI2CON = 0b0000001001100000; // SMP,CKE
    SPI2CON2 = 0;
    SPI2STAT = 0;
    SPI2BRG = (clockfreq / 2) / sdclk - 1; // 0 = 24MHz
#if((clockfreq/2)/sdclk) * sdclk!=clockfreq/2
#warning invalid sdclk divider value
#endif
    SPI2CONbits.ON = 1;
#if debug_dma==0
    iosetup_sd;
#endif
    I2C2CON = 0x1200;
    I2C2BRG = clockfreq / 2 / i2cspeed_norm; //350000; //350k - something starts flaking out around 400K - I2C peripheral appears to get into odd state & hang
    I2C2CON = 0x9200;

    delayus(10000); // allow supply stabilise time

    oled_init();
    //acc_init(); // do after selftest in case of jam due to bus problem

    // pulse camera powerdown in case it's jamming the i2C bus - it sometimes does this if underclocked
    oledcmd(0x1b5);
    oledcmd(0x0f);
    delayus(100);
    oledcmd(0x1b5);
    oledcmd(0x0C);

}

void readbatt(void) {
    unsigned int i, r;
    static unsigned int battave = 0, startcount = 0;
    if(adcclaimed) {battlevel=4000;return;}

    // measure battery voltage
    __builtin_disable_interrupts(); // prevent powercon-off time being extended. Only for about 3.5uS so shouldn't break anything
    powercon_off;
    AD1CHS = readbatchan << 16;
    AD1CON1bits.SAMP = 1;
    while (AD1CON1bits.SAMP); // wait til sample done
    powercon_on; // turn power back on as quick as possible
    __builtin_enable_interrupts();
    while (AD1CON1bits.DONE == 0);
    i = ADC1BUF0;

    AD1CHS = 0xE0000; // measure internal vref (can't relay on 2.8v Vcc in case 2v8 reg has dropped out)
    AD1CON1bits.SAMP = 1;
    while (AD1CON1bits.DONE == 0);
    r = ADC1BUF0;

    i = i * 0x10000 / r; // measure relative to reference
    if (++startcount < 10) battave = i; //initialise avaraging filter at startup
    battave = (battave * 255 + i) / 256; // IIR filter to avarage out noise etc.
    battlevel = battave * 3000 / 89800; // scale to mV - 89800= measured value at 3000mV in

}

void readbuttons(void) {
    unsigned int i;
   
    
    // discovered that some SD cards have weak pullup on Din, need to fudge around this by reading but4 at right time - 
    // slow enough for 2k2 switch pullup, but not so slow we catch the slower rise of the sd pullup  
    __builtin_disable_interrupts();
   butts_in; // shared IOs to inputs
   butpress = butstate; 
   butstate = 0;
   Nop();Nop();Nop();Nop();Nop();Nop();Nop();
   if (butin_4) butstate |= but4;
   __builtin_enable_interrupts();
    if (butin_3) butstate |= but3;
    delayus(10);// settle time for other pulldowns
         
    if (butin_1) butstate |= but1;
    if (butin_2) butstate |= but2;
    
    
    if (butin_5) butstate |= but5;
    if (powerbut_in) butstate |= powerbut;
    butpress = (butpress ^ butstate) & butstate; // changed and now down
    butts_out;
   
    if (butstate & butmask) reptimer++;
    else reptimer = 0; // exclude power but from autorepeat
    if (reptimer == reptime) {
        butpress = butstate & butmask;
        reptimer -= reprate;
    } // assumes called every tick and ticks are uniform

}

void enterboot(void) { // enter bootloader by jumping to reset address
    cam_enable(0);
    __builtin_disable_interrupts();
    void (*fptr)(void);
    fptr = (void (*)(void))(0xBFC00000);
    fptr();
}

void selftest(void) {
    unsigned int i, c, d, p, e, b, lb, lc, j;

    do {
        p = 0;
        //1    .1    .1    .1    .1    .1    .1    .1    .1    .1    .1    .1    .1    .1    .
        const char campins[] = " 6/10  4/9   3/8   5/1   7/44  9/43 11/42 13/41  8/4   8/5  18/3              16/37";
        led1_on;
        printf(bgred cls "Selftest" del bggrn cls del bgblu cls del whi cls "Accelerometer ");
        led1_off;
        i = 0;
        I2C2CONbits.SEN = 1;
        while ((I2C2CONbits.SEN) && (++i < 10000));
        I2C2TRN = 0;
        while ((I2C2STATbits.TBF)&&(++i < 10000));
        if (I2C2STATbits.BCL) i = 10000;
        while ((I2C2STATbits.TRSTAT)&&(++i < 10000));
        I2C2CONbits.PEN = 1;
        while ((I2C2CONbits.PEN) && (++i < 10000));
        if (i >= 10000) {
            printf("\nI2C bus Error\n");
            goto testfail;
        }
        i = iistart(iicadr_acc);
        iistop();
        if (i == 0) {
            printf("\nAccel Ack Error\n");
            goto testfail;
        }
        i = iistart(iicadr_acc);
        iisend(0x0f);
        iirestart(iicadr_acc + 1);
        i = getiic(0);
        iistop();
        if (i != accel_id) {
            printf("\nAccel ID Error\n");
            goto testfail;
        }
        printf("OK\nCamera ");
        i = (iistart(iicadr_cam));
        iistop();
        if (i == 0) {
            printf("\nCam I2C/XCLK Error\n");
            goto testfail;
        }

        cam_enable(cammode_128x96_z1);

        if (readcamreg(0x0a) != 0x96) {
            printf("\nCam ID Error\n");
            goto testfail;
        }

        //cam_setreg(0x8d,0x12); // colour bar 
        c = PORTC;
        b = PORTB;
        d = 0;
        e = 0;
        for (i = 0; i != 0x300000; i++) {// check for transitions on cam data/sync/picxlk
            lc = c;
            lb = b;
            c = PORTC;
            b = PORTB;
            d |= (b^lb);
            e |= (c^lc);
        }
        led1_off;

        d = ((d & 0x1fe0) << 1) | (e & 0x390) >> 4;
        if (d != 0x3ff9) {

            printf("\nCam pin fault:\nCam pin / PIC pin\n");
            for (i = 0x2000, e = 0; i; i >>= 1, e += 6) if ((i & d) == 0) for (j = 0; j != 6; dispchar(campins[e + (j++)]));
            printf("\n");
            goto testfail;
        }
        printf(" OK\n");
        readbatt();
        if ((battlevel < 2500) || (battlevel > 4000)) {
            printf("Batsense fault %d\n", battlevel);
            goto testfail;
        }

        sd_cs_in;
        delayus(5);
        i = sd_carddet;
        sd_cs_out;
        printf("SD Card ");

        if (!i) printf(inv "No Card\n" inv);
        else {
            if (FSInit()) printf("OK\n");
            else {
                printf("FAIL");
                goto testfail;
            }
        }

        printf("SRAM ");
        sram_cs_lo;
        sendspi(2);
        sendspi(0);
        sendspi(0);
        sendspi(0);
        sendspi(0xF5); //write, address, data
        sram_cs_hi;
        sram_cs_lo;
        sendspi(3);
        sendspi(0);
        sendspi(0);
        sendspi(0); //address
        i = sendspi(0);
        sram_cs_hi;
        if (i == 0xf5) printf("OK\n");
        else {
            printf("FAIL\n");
            goto testfail;
        }
/*
        readbuttons();
        if (butstate) {
            printf("Stuck button %X", butstate);
            goto testfail;
        }
*/     butpress=0;
        printf(yel "Press each button\n" whi);
        b = 0;
        do {
           reptimer=0; 
            readbuttons();
            b |= butpress;
            printf(tabx0 taby5);
            for (c = 1, i = 1; i != 7; i++, c <<= 1) {
                printf("But %d ", i);
                printf((b & c) ? "OK\n" : "  \n");
            }
        } while (b != (butmask | powerbut));

        p = 1;
testfail:
        if (p) printf(grn inv "        PASS        " inv whi);
        else printf(red inv "\n        FAIL        " inv whi);
        do {
            delayus(20000);
            readbuttons();
        } while (butstate);
        printf(bot butcol "EXIT           Repeat");
        do {
            delayus(20000);
            readbuttons();
        } while (butpress == 0);
    } while (butpress & but3);
    printf(whi cls);

}


//_______________________________________________________________________________ NVM control

//#define NVMCON_WR (1<<15)
//#define NVMCON_WREN (1<<14)
#define pagesize 0x400 // PIC32 flash parameters
#define rowsize 0x80
#include <sys/kmem.h>
void nvm_read(void)
{
 // read NVM area into nvmbuf
   unsigned int i, *cptr;
       cptr=(unsigned int *) (nvm_addr | 0xA0000000); // physical to logical address
       for(i=0;i!=nvm_size/4;i++) nvmbuf.words[i]=*cptr++;
    
}


void __attribute__((nomips16)) do_flash(unsigned long op)
{

 NVMCON = NVMCON_WREN | op;// page erase
 delayus(10); // settle time
  NVMKEY 		= 0xAA996655;
    NVMKEY 		= 0x556699AA;
    NVMCONSET 	= NVMCON_WR;
    while(NVMCON & NVMCON_WR);
    NVMCONCLR = NVMCON_WREN;  
  
}
void nvm_write(void)
{
    unsigned int i,p;
   __builtin_disable_interrupts();
  NVMADDR=nvm_addr;
  do_flash(4); // erase page
  
  for(p=0;p!=pagesize;p+=rowsize) 
  
  {
   NVMADDR=nvm_addr+p;
   NVMSRCADDR = KVA_TO_PA(&nvmbuf.bytes[p]);

  do_flash(3); // row program
  }
  
  
  __builtin_enable_interrupts(); 
    
    
}

    void reboot(void)
    {
    // perform a reset
      __builtin_disable_interrupts();  
    SYSKEY = 0x00000000; //write invalid key to force lock
     SYSKEY = 0xAA996655; //write key1 to SYSKEY
     SYSKEY = 0x556699AA; //write key2 to SYSKEY
     /* set SWRST bit to arm reset */
     RSWRSTSET = 1;
     /* read RSWRST register to trigger reset */
      if(RSWRST);
     /* prevent any unwanted code execution until reset occurs*/
     while(1);
     }


//=====================================================================================