

#ifndef _HARDWAREPROFILE_H_
#define _HARDWAREPROFILE_H_
#endif



        #define GetSystemClock()            clockfreq              // System clock frequency (Hz)
        #define GetPeripheralClock()        clockfreq              // Peripheral clock frequency
        #define GetInstructionClock()       (GetSystemClock())      // Instruction clock frequency
   

    // Clock values

    #define MILLISECONDS_PER_TICK       10                  // Definition for use with a tick timer
    #define TIMER_PRESCALER             TIMER_PRESCALER_8   // Definition for use with a tick timer
    #define TIMER_PERIOD                37500               // Definition for use with a tick timer



#define SD_CD (sd_carddet)
#define SD_CS (sd_cs)


#define USE_SD_INTERFACE_WITH_SPI


/*********************************************************************/
/******************* Pin and Register Definitions ********************/
/*********************************************************************/

/* SD Card definitions: Change these to fit your application when using
   an SD-card-based physical layer                                   */


// enable Mike's quick-hack optimisations. Increases read speed by approx 2x
#define optimise_sd_read 0 // speed up copy from sector buffer to destination by not checking for end of cluster every byte. 
                           // reduces copy time from 800 to 250uS at 48MHz. Approx 50% overall speed improvement. NB messes up EOF detection
                           // still scope to improve using word copy or DMA, but diminishing returns. 
 #define optimise_spi32 1 //  use 32 bit transfers to improve speed. reduces block-read from 680 to 520uS @ 48MHz(24MHz SPI CLK)


#define spiclk_slow 63
#define spiclk_fast 0 // 0=20MHz, 1=10MHz
 
        // Registers for the SPI module you want to use
        #define MDD_USE_SPI_1
    

		//SPI Configuration
		#define SPI_START_CFG_1     //(PRI_PRESCAL_64_1 | SEC_PRESCAL_8_1 | MASTER_ENABLE_ON | SPI_CKE_ON | SPI_SMP_ON)
        #define SPI_START_CFG_2     // (SPI_ENABLE)

        // Define the SPI frequency
        #define SPI_FREQUENCY			(sdclk)
    

      
            // Description: The main SPI control register
            #define SPICON1             SPI2CON
            // Description: The SPI status register
            #define SPISTAT             SPI2STAT
            // Description: The SPI Buffer
            #define SPIBUF              SPI2BUF
            // Description: The receive buffer full bit in the SPI status register
            #define SPISTAT_RBF         SPI2STATbits.SPIRBF
            // Description: The bitwise define for the SPI control register (i.e. _____bits)
            #define SPICON1bits         SPI2CONbits
            // Description: The bitwise define for the SPI status register (i.e. _____bits)
            #define SPISTATbits         SPI2STATbits
            // Description: The enable bit for the SPI module
            #define SPIENABLE           SPI2CONbits.ON

            // Description: The definition for the SPI baud rate generator register (PIC32)
            #define SPIBRG			    SPI2BRG




