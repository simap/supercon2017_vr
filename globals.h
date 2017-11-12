// global variables, macros and function prototypes
// also documentation on global variables and useful functions

//_____________________________________________________________________ camera status

extern volatile unsigned char cam_started; // set at start of first active line, cleared by app code
extern volatile unsigned char cam_busy; // set on vsync, cleared when all lines captured
extern volatile unsigned char cam_newframe; /// set when all lines captured, cleared by app code
extern volatile unsigned char cam_wrap; // set if vsync happens when still grabbing
extern volatile unsigned char cam_stop; // set to stop after current frame

extern volatile unsigned int xstart, ystart; // capture start position in frame.Sampled at vsync
extern unsigned int xdiv, ydiv; // pixel clock/line divider. Sampled at vsync
extern unsigned int xpixels, ypixels; // number of pixels to capture. Sampled at vsync
extern unsigned int camaddr; // current capture address (offset within cambuffer), updated at end of each line  
extern volatile unsigned int linecnt; // current capture line used by FG task to track capture progress & send lines as soon as stored 
extern unsigned int cammode; // current mode - used to detect changes 
extern unsigned int camflags; // current camera flags. Loaded from camconfig[cammode] 
extern unsigned int camoffset; // capture start address offset from start of cambuffer. Sampled at vsync
extern unsigned int cammax; // limit of camera memory - will wrap round 

// _____________________________________________________________________________serial

#if serialcontrol==1
extern volatile int rxhead, rxtail, rxtimer;
extern unsigned char rxbuf[rxbufsize];
#endif
//___________________________________________________________________________________________ general memory  
// 
// union to allow buffer access by bytes or words 

typedef union {
    unsigned char bytes[cambufsize];
    unsigned short shorts[cambufsize / 2];
    unsigned long words[cambufsize / 4];
} buffertype;

extern buffertype buffer_union;
#define cambuffer buffer_union.bytes
#define cambuffer_s buffer_union.shorts
#define cambuffer_w buffer_union.words

// alias buffer union to simplify syntax
// access as bytes : cambuffer[0..cambufsize-1] 
// access as shorts : cambuffer_s[0..cambufsize/2-1]
// access as words : cambuffer_w[0..cambufsize/4-1]

/*
 if your application needs a static array of something other than bytes, shorts or words, to avoid wasting memory,
 overlay it with cambuffer as follows. For example if you want an array of these structs 

    typedef struct {
    unsigned int a;
    unsigned int b;
    unsigned int c;
} triple;
 
which you'd normally declare like this : 

static triple trips[100];

 instead, declare it like this, so it gets overlayed with the cambuffer memory : 
 
triple *trips = (triple*) &cambuffer[0];

 Obviously you can use offsets into cambuffer if you want to use some of cambuffer, or need multiple arrays
 Take care not to use more than cambufsize bytes total
  
 */


// another small general-purpose buffer for AVI header parsing and building, also pallette for BMP files
// free for use by applications as bytes, shorts or words

typedef union {
    unsigned char bytes[hbuflen];
    unsigned short shorts[hbuflen / 2];
    unsigned int words[hbuflen / 4];
} headerbuftype;

extern headerbuftype headerbuf;
#define avibuf headerbuf.bytes
#define palette headerbuf.shorts
#define nvmbuf headerbuf

//________________________________________________________________________________ structs for filesystem

extern FSFILE * fptr; // file system struct for access to current open file
extern SearchRec searchfile, searchdir; // struct for listing files with FSfindfirst

//________________________________________________________________________________ misc global state info 

extern unsigned char butstate; // buttons currently down button bits defined in cambadge.h
extern signed int accx, accy, accz; // accelerometer values. range +/-16000
extern unsigned int battlevel; // battery level in mV
extern unsigned char cardmounted; // =1 if card is inserted and filesystem available 
extern unsigned int powerdowntimer; // only global as we want to reset it on serial commands as well as buttons/accel move. 
// Zero this if you want to prevent powerdown
unsigned int reptimer; // auto-repeat timer - set this to zero to disable auto-repeat

//event flags set once on poll
extern unsigned char butpress; // buttons pressed - bits as per butstate
extern unsigned char cardinsert; // cardmountd indicates file ops can be done. cardinsert is insert/remove event flag set once in polling loop
extern unsigned int tick; // system ticks normally set to 1 every 20ms, but if apps take longer, count will reflect approx elapsed ticks since last call

//display 

extern unsigned char dispx, dispy; // current cursor x,y position ( pixels) 0..127
extern unsigned short fgcol, bgcol; // foreground and background colours , RGB565

extern unsigned char dispuart; // flag to divert printf output to UART2 for debugging 0 = normal, 1 = UART 1, 2 = UART 2
// set bit 4 to output to serial and screen

unsigned char adcclaimed; //=1 if someone is using the ADC so disable battery reads

// video /bmp file parameters used by stuff in fileformats.c

extern unsigned int avi_width, avi_height, avi_bpp; // width,height in pixels, bytes per pixel (1,2 supported for record, 1,2,3 for playback)
extern unsigned int avi_frametime, avi_framelen, avi_frames; //uS per frame, bytes per frame, number of frames
extern unsigned int avi_framenum, avi_start; // current frame number, file offset of image data of first frame (after 00dc chunk header)

//___________________________________________________________________________ function prototypes

//________________________________________________________________________ stuff used internally, applications shouldn't need to use these

void inithardware(void); // set up all peripherals etc.
void setupints(void); // set up interrupts
void oledcmd(unsigned int); // Send byte to OLED - b8 set for command, clear for data
void oled_init(void); // initialise oled
void acc_read(void); // read accelerometer
void polluart(void); // poll for serial commands
void enterboot(void); //enter bootloader
void readbatt(void); // read battery voltage
unsigned char readcamreg(unsigned char c); // read camera register
void selftest(void); // hardware selftest

//_____________________________________________________________________ stuff that might be useful for applications

void cam_setreg(unsigned char, unsigned char); // write camera register
void dispchar(unsigned char); // display character or do control code. normally used via printf
void do_delay(unsigned int); // delay in cycles, used by delayus macro
void u1txbyte(unsigned int c); // send byte on UART 1. Enables UART1 and maps pins on first call
void u2txbyte(unsigned int c); // send byte on UART 2
void u2txword(unsigned int c); // send 2 bytes LSB first
void nvm_read(void); // read NVM flash page into nvmbuf[] ( access as nvmbuf.bytes[0..nvm_size], nvmbuf.shorts[0..nvm_size/2],nvmbuf.words[0..nvm_size/4],
void nvm_write(void); // erase and write NVM flash page from nvmbuf{}
void reboot(void); // reset the processor. This will present the "enter bootloader" option then restart the main application code
int randnum(int min, int max); // return signed random number between ranges


unsigned int iistart(unsigned char iiadr); //  Starts I2C command, iiadr - device address ( 8 bit). Returns zero if device does't acknowledge
unsigned int iisend(unsigned char byte); // Sends 1 byte, returns zero if no acknowledge
void iirestart(unsigned char iiadr); // Send Restart condition for read operation unsigned
unsigned int getiic(unsigned int ack); //Read 1 byte, ack=1 to acknowledge ( set to 0 for last byte ).
void iistop(void); //   Send I2C stop condition

void dispimage(unsigned int xstart, unsigned int ystart, unsigned int xsize, unsigned int ysize, unsigned int format, unsigned char* imgaddr);
// display images in various formats. See cambadge.h for options
// for 1 byte/pixel, the palette must be set up first  e.g. using monopalette(). Note this shares memory with the AVI header buffer
// format options defined in cambadge.h

void plotblock(unsigned int xstart, unsigned int ystart, unsigned int xsize, unsigned int ysize, unsigned int col);
// plot block of solid colour (RGB565) use rgbto16 macro to convert from r,g,b, or fixed colours in primarycol[]

void mplotblock(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int colour, unsigned char* imgaddr);
// as plotblock but plots into memory buffer for subsequent display using dispimage
// faster if plotting liots of small pixels. Note no bounds checks so will BSOD with bad parameters
// only supports 8 bits/pixel

void monopalette(unsigned int min, unsigned int max);
// set up palette for mono images to greyscale between min and max

unsigned int showavi(void);
// display next  frame of AVI previously opened with openavi. rewinds to avi_start if avi_framenum>=avi_frames

unsigned int loadbmp(char*, unsigned int);
// read BMP file 0 : just get info, 1 : load into cambuffer 2 : load and display

unsigned int writebmpheader(unsigned int xsize, unsigned int ysize, unsigned int bpp);
// write a BMP header (and pallette table for mono) to open file 

void flipcambuf(unsigned int xpixels, unsigned int ypixels, unsigned int offset);
// vertical flip image in camera buffer for mono AVI. Also changes greyscale range to 16-240

unsigned int startavi(void); // Start AVI write - just writes dummy header, only needs avi_bpp
unsigned int finishavi(void); // write index and header 


void cam_enable(unsigned int mode);
// initialises or disables camera with parameters for specified mode. Does not start grabbing until grabenable used 

void cam_grabdisable(void);
// suspends grab process, camera stays initialised

void cam_grabenable(unsigned int opt, unsigned int bufoffset, unsigned int cambuflen);
// starts acquisition. pptions in cambadge.h
// first byte is garbage, so cam data will be at cambuffer[bufoffset+1]. See cambadge.h for modes
//cambuflen sets value at which acquisition address will wrap to bufoffset, =0 for maximum. Can be used for FIFO acquisition of large frames 

void conv16_24(unsigned int npixels, unsigned int offset);
// convert image at cambuffer[offset] from RGB565 to RGB888

void claimadc(unsigned char claim); // call with 1 to claim use of ADC, disables battery read. Call with 0 to release

//________________________________________________________________________________________ macros 

#define delayus(d) do_delay((unsigned long)d*(clockfreq/1000000)) // done as macro  so scaling done at compile time
#define filetype(a,b,c) ((a<<16) | (b<<8) | c) // convert e.g. 'A','V','I' to word for filetype comparison. saves defining constants for all filetypes
#define kickwatchdog WDTCONSET=1  // kick the dog
#define rgbto16(r,g,b) (((r)&0xF8)<<8 | ((g) & 0xfc)<<3 | ((b)&0xf8)>>3) // convert RGB8,8,8 to RGB565

//_____________________________________________________________ misc tables

const char* avierrors[] = {"None", "Not found", "Read Err", "Not an AVI", "LIST Error", "Hdr Err", "Strm Err", "MOVI Err", "00dc Err", "Frame too big", "Unknown format", "Frame too wide", "Frame too tall",
    "Not a BMP"};

// lookup for primary colors, and dim primaries        
#define dim 156 
const unsigned short primarycol[16] = {
    rgbto16(0, 0, 0), rgbto16(255, 0, 0), rgbto16(0, 255, 0), rgbto16(255, 255, 0), rgbto16(0, 0, 255), rgbto16(255, 0, 255), rgbto16(0, 255, 255), rgbto16(255, 255, 255),
     rgbto16(0, 0, 0), rgbto16(dim, 0, 0), rgbto16(0, dim, 0), rgbto16(dim, dim, 0), rgbto16(0, 0, dim), rgbto16(dim, 0, dim), rgbto16(0, dim, dim), rgbto16(dim,dim,dim)
};