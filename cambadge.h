#include <xc.h>
#include <stdio.h> // for printf
#include "pindefs.h" // IO pin defs and access macros
#include "appmap.h" // main menu configuration
#include "MDD_File_System/FSIO.h"
#include <sys/attribs.h> // Interrupt vector constants 

//=========================================================== 
// debug and high-level tweak options
#define version "BADGE 1.03"
#define versionbyte 103 // returned by serial command


#define uart1_enable 1 // ==1 to set up UART1 and set pins for UART mode
#define u1baud  19200  
#define serialcontrol 1 // =1 to enable serial control via ttl232 header
#define u2baud  100000 // TTL232 header for debug etc.

#define ticktime 20000 // tick time, uS
#define splashtime 120  // time splasheceren held 9 after playback if avi)
#define accmovethresh 1000 // not-moving threshold for accelerometer
#define powerdowntime 30000 // no-activity powerdown time, ticks
#define powerbuttime 100 // length of power button press before powerdown, ticks
#define reptime 25 // key auto repeat delay, 20ms ticks
#define reprate 4 // auto repeat rate
#define butcol cya // button legend colour
#define batthresh1 2800 // yellow bat thresh, mV
#define batthresh2 2200 // red bat thresh, mV

#define debug_dma 0 // =1 to enable PMRD signal on pin 11 to see cam DMA reads. SD card can't be used if enabled

//============================F==============================================

// RAM buffers
#define cambufsize (dispwidth*dispheight*3+256) // size of image buffer, to fit rgb888 plus a little overlap 
#define hbuflen 1024 // size of buffer used for pallette and avi/bmp header noodling, also buffer for NVM data
#define rxbufsize 256 // serial control rx buffer size

#define nvm_addr 0x1D007C00 // flash address of NV memory
#define nvm_size 0x400 // bytes
//_____________________________________________________________________ 
#define dispwidth 128
#define dispheight 128

#define oled_upscan 1 // =1 for oled scan bottom to top to match BMP,AVI format. used by oled init and font/block plotting code
                      // only left here as it's changed a few times previously,
                      // but probably won't again
//flags for dispuart 
#define dispuart_off 0
#define dispuart_u1 1 // send printf to uart1
#define dispuart_u2 2 // send printf to uart 2
#define dispuart_screen 0x10 // bot to send printf to uart and screen
//_____________________________________________  camera option bits. 
// loaded into camflags from cammode by cam_enable

#define camopt_vga 1              //=1 for vga, =0 for qvge
#define camopt_mono 2             //=1 for mono, 0 for RGB565
#define camopt_clkphase 4         //=1 to invert pixclk phase
#define camopt_swap 8             // =1 to swap bytes
#define camopt_refclk_2 0x00      // 0,1,2,3  for clkdiv 2,3,4,5
#define camopt_refclk_3 0x20
#define camopt_refclk_4 0x40
#define camopt_refclk_5 0x60
#define camopt_double 0x80   // two-byte DMA cell, for RGB565 or highest speed mono

//cam_grabenable options

#define camen_start 1 // start and exit immediately
#define camen_grab 2 // enable, grab one frame,stop

// camera modes for camenable()

#define cammode_off 0
#define cammode_128x96_z1 1
#define cammode_128x96_z2 2
#define cammode_128x96_z1_mono 3
#define cammode_128x96_z2_mono 4
#define cammode_128x96_z4_mono 5

typedef struct {
    unsigned short xpixels;
    unsigned short ypixels;
    unsigned char xdiv;
    unsigned char ydiv;
    unsigned short xstart;
    unsigned short ystart;
    unsigned short flags;
} camconftype;

#define ncammodes 6
#define fastcolzoom 1 // faster x2 colur zoom, timing might be a bit sketchy

const camconftype camconfig[ncammodes]={ 
{128,96,4,2,0,0,0},
{128,96,4,2,30,24,camopt_refclk_2 | camopt_double },
#if fastcolzoom==1
{128,96,4,2,188,144,camopt_refclk_3 | camopt_vga | camopt_clkphase | camopt_double },
#else
{128,96,4,2,176,144,camopt_refclk_4 | camopt_vga | camopt_swap}, 
#endif
{128,96,4,2,30,24, camopt_refclk_2 | camopt_mono | camopt_swap },
{128,96,2,1,96,73,camopt_refclk_2 | camopt_mono | camopt_swap },
{128,96,2,1,224,194,camopt_refclk_3 | camopt_vga | camopt_mono | camopt_swap }

};

const char* camnames[ncammodes] = {"", "128x96 x1 RGB", "128x96 x2 RGB", "128x96 x1 B/W", "128x96 x2 B/W", "128x96 x4 B/W"};

//_________________________________________________________________hardwareish stuff


#define clockfreq 48000000UL // 
#define refclkdiv 2 // default camera clock divider 1=24MHz, 2=12MHz 3 = 8MHz 4 = 6MHz

#define t4prescalebits 7
#define t4prescale (2<<t4prescalebits) //256 tick timer prescale

//____________________________________________________________________ I2C 

#define accel_dh 0
#define accel_hh 1

#define acceltype accel_hh // prototypes had LIS2DH12, produciton have HH

#if acceltype==accel_hh
#define accel_id 0x41
#define iicadr_acc 0x3a
#endif

#if acceltype==accel_dh // prototype

#define accel_id 0x33
#define iicadr_acc 0x32
#endif

#define iicadr_cam 0x60 // 8 bit addresses


#define i2cspeed_norm 300000 // i've seeen odd issues on other projects with >350k so be save
#define i2cspeed_cam 100000 // slow I2c for cam as underclocking messes up I2C 

// application action codes

#define act_name 0
#define act_init 1
#define act_help 2
#define act_start 3
#define act_poll 4
#define act_powerdown 5

//____________________________________________ butstate/butpress bits

#define but1 1
#define but2 2
#define but3 4
#define but4 8
#define but5 16
#define powerbut 0x20
#define butmask (but1|but2|but3|but4|but5) // non-power buttons, used to mask auto-repeat

//__________________________________________________________________________ display stuff

//image formats for dispimage. NB assumes format number b0,1 = bytes per pixel so need tweaks if more formats added

#define img_mono 1 
#define img_rgb565 2
#define img_rgb888 3
#define img_revscan 0x04 // reverse vertical scan direction
#define img_vdouble 0x08 // double vertical pixels
#define img_skip1 0x10 // skip alternate pixels for downsampling 
#define img_skip2 0x20
#define img_skip3 0x30 
#define img_skip4 0x40 
#define img_skip5 0x50 
#define img_skip6 0x60 
#define img_skip7 0x70 
#define img_skip8 0x80 
#define img_skip9 0x90 
#define img_skip10 0xa0 


//________________________________________ text 

// control chars etc. 
// 0..0x0f : control codes
// 0x10..0x17 : reserved for more control codes or printable chars
// 0x18..0x1f : custom chars 
// 0x20..0x7f : Standard ASCII
// 0x80..0x93 : X tab
// 0xa0..0xaf : y tab
// 0xc0..0xFF : set text colour 11bbbfff bbb= background, fff = foreground

#define charwidth 6
#define charheight 8
#define vspace 10 // character vertical spacing


// Control character strings for display 
#define del "\x02" // 500mS delay
#define hspace "\x03" // half space
#define bspace "\x04" // 2 pixel backspace
#define inv "\x07" // swap fg/bg colours
#define cls "\x0c" // clear screen
#define lf "\x0a" // CR and LF
#define cr "\x0d"  // CR only
#define grey "\x0e" // grey  fg colour

//special display characters
// add more below 0x18 and update startchar in font6x8.inc
#define shortdot "\x1e\x04" 

#define uarr "\x18" // arrows
#define darr "\x19"
#define larr "\x1a"
#define rarr "\x1b"
#define bat "\x1c" // battery
#define sdcd "\x1d" // SD card
// 0x1e,0x1f spare
// tabs
#define tabx0  "\x80"
#define tabx1  "\x81"
#define tabx2  "\x82"
#define tabx3  "\x83"
#define tabx4  "\x84"
#define tabx5  "\x85"
#define tabx6  "\x86"
#define tabx7  "\x87"
#define tabx8  "\x88"
#define tabx9  "\x89"
#define tabx10 "\x8a"
#define tabx11 "\x8b"
#define tabx12 "\x8c"
#define tabx13 "\x8d"
#define tabx14 "\x8e"
#define tabx15 "\x8f"
#define tabx16 "\x90"
#define tabx17 "\x91"
#define tabx18 "\x92"
#define tabx19 "\x93"

#define taby0 "\xa0"
#define taby1 "\xa1"
#define taby2 "\xa2"
#define taby3 "\xa3"
#define taby4 "\xa4"
#define taby5 "\xa5"
#define taby6 "\xa6"
#define taby7 "\xa7"
#define taby8 "\xa8"
#define taby9 "\xa9"
#define taby10 "\xaa"
#define taby11 "\xab"
#define taby12 "\xac"

#define bot "\x80\xac"
#define top "\x80\xa0"

// colours b0..2 = foreground, b5..3 = background
#define red "\xc1"
#define grn "\xc2"
#define yel "\xc3"
#define blu "\xc4"
#define mag "\xc5"
#define cya "\xc6"
#define whi "\xc7"
#define bgred "\xcf"
#define bggrn "\xd7"
#define bgblu "\xe7"


#define whiinv "\xF8"

// primary colours

#define c_blk 0
#define c_red rgbto16(255,0,0)
#define c_grn rgbto16(0,255,0)
#define c_yel rgbto16(255,255,0)
#define c_blu rgbto16(0,0,255)
#define c_mag rgbto16(255,0,255)
#define c_cya rgbto16(0,255,255)
#define c_whi rgbto16(255,255,255)
#define c_grey rgbto16(168,168,168)



