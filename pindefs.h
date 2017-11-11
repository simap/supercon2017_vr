// define pin functions and hardware-specific things like peripheral clocks  
// as there are various ways of accessing IO, the needed functionality is done using macros 
// note there are lots of limitations due to pin remapping constraints etc.
// note that due to silicon errata, (non-RP) IO output on pins shared with PMADDR function 
// must be done by enabling the bit in PMAEN and setting the pin state via PMADDR

// front LED
// use xxSET/CLR for IO ports to reduce risk of bad things if interrupt tasks ever get used for IO access

#define led1 LATAbits.LATA3
#define led1_on LATACLR=1<<3
#define led1_off LATASET=1<<3
#define iosetup_leds TRISACLR=1<<3;ODCAbits.ODCA3=1; // open-drain to avoid glow at high supply voltages

// buttons, active high (active high due to possible inadequate internal CNPUx pullup - see device errata)
// button inputs are shared with OLED controls and SD Dout, so only set to input when reading

#define iosetup_butts CNPDAbits.CNPDA8=1; CNPDCbits.CNPDC0=1;CNPDCbits.CNPDC1=1;CNPDAbits.CNPDA9=1;TRISAbits.TRISA9=1;
#define butin_1 PORTCbits.RC0 // disp bot left
#define butin_2 PORTCbits.RC1 // display centre
#define butin_3 PORTAbits.RA8 // display bot right
#define butin_4 PORTAbits.RA1 // display top right
#define butin_5 PORTAbits.RA9 // top trigger

// set button pins to input for reading
#define butts_in SPI1CONbits.ON=0;SPI2CONbits.ON=0;TRISCSET=1<<0 | 1<<1;TRISASET= 1<<1 | 1<<8 | 1<<9;
// restore button pins to primary functions.
#define butts_out TRISCCLR=1<<0|1<<1;TRISACLR=1<<1 | 1<<8  ;SPI1CONbits.ON=1;SPI2CONbits.ON=1;

// power control and battery measure

#define powercon_on TRISACLR=1<<4;LATASET=1<<4;
#define powercon_off LATACLR=1<<4;// power enable, active high, also battery measure when pulsed low briefly
#define powerbut_in PORTAbits.RA7 // power button
#define iosetup_powerbut TRISASET=1<<7;TRISACLR=1<<4;CNPDAbits.CNPDA7=1;
#define iosetup_readbat TRISCSET=1<<2;ANSELCbits.ANSC2=1;


#define readbatchan 8 // ADC channel for reading battery voltage

//UART1 - expansion header u1tx c5 u1rx c3 
#define iosetup_uart1 U1RXRbits.U1RXR=7; RPC5Rbits.RPC5R=1;CNPUCbits.CNPUC3=1;// RB1 Rx RB0 TX, pullup on RXD  

//UART2 - TTL232 header
#define iosetup_uart2 U2RXRbits.U2RXR=2; RPB0Rbits.RPB0R=2;CNPUBbits.CNPUB1=1;// RB1 Rx RB0 TX, pullup on RXD  

// camera
#define iosetup_cam  T3CKRbits.T3CKR=6; \
    RPC6Rbits.RPC6R=7;INT4Rbits.INT4R=5;INT1Rbits.INT1R=7;    // T5 pixclk, t3 pixclk2 RC6 refclk,INT4 Vsync INT1 Hsync

// SD card
#define iosetup_sd TRISACLR=1<<0;CNPDAbits.CNPDA0=1;CNPUBbits.CNPUB13=1; \
                   RPA1Rbits.RPA1R=4;SDI2Rbits.SDI2R=3; //A1 = SDO2, B13 = SDI2, CS pulldown for card det

#define sd_cs  LATAbits.LATA0 // sd cs
#define sd_carddet PORTAbits.RA0 // sd card detect - shared with CS. CarDdet switch pulls high against internal pullup when card present
#define sd_cs_in TRISASET=1<<0 // CS as input to read card-detect
#define sd_cs_out TRISACLR=1<<0 // CS as output for card access

#define sdclk 12000000 // SD card SPI clock. must be (clockfreq/2)/integer
#define cardmounttime 5 // number of ticks card detect debounce
// CS out, pulldown for card detect ,pullup on din


// OLED display
#define iosetup_oled TRISCCLR=1<<0 | 1<<1;TRISACLR= 1<<10 | 1<<8 ;RPA8Rbits.RPA8R=3; PMAEN|=1<<10; //enable PMA10 for errata fix

#define oled_cd_lo LATCCLR=1<<0
#define oled_cd_hi LATCSET=1<<0
#define oled_cs_lo LATCCLR=1<<1
#define oled_cs_hi LATCSET=1<<1
#define oled_rst_hi PMADDR|=1<<10; // See errata - IO shared with PMP Address outputs doesn't work as output, so need to use PMADDR
#define oled_rst_lo PMADDR&=~(1<<10); //

#define sram_cs_lo LATCCLR=1<<1 // shared with oled
#define sram_cs_hi LATCSET=1<<1

#define oledclk 12000000 // OLED SPI clock. must be (clockfreq/2)/integer
#define oled_reset_time 50 //uS
