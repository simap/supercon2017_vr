#include "cambadge.h"
#include "MDD_File_System/FSIO.h"
// all global variables declared here for easy reference. 
// globals.h includes extern declarations - see that file for info on variable usage

volatile unsigned char cam_started, cam_vsync, cam_busy, cam_newframe, cam_wrap,cam_stop;
volatile unsigned int linecnt, camaddr; // camera status
volatile unsigned int xstart, ystart, xdiv, ydiv, xpixels, ypixels; // camera parameters etc.
unsigned int cammode, camflags, camoffset;
unsigned int cammax;

#if serialcontrol==1
volatile unsigned int rxptr = 0, rxtimer; //  UART2 receive pointer & framing timeout
unsigned char rxbuf[rxbufsize]; // UART2 receive buffer
#endif

unsigned char dispx, dispy; // display cursor position ( pixels))
unsigned short fgcol, bgcol; //foreground & background colours 
unsigned int reptimer=0; // auto-repeat timer - set this to zero to disable auto-repeat
unsigned char butstate, butpress; // current button state, and button-changed flags
signed int accx, accy, accz; //last accelerometer data

unsigned char dispuart = 0; // where to send PRINTF/dispchar =0 for display, 1/2 for uart1/2
unsigned char adcclaimed = 0; //=1 if someone is using the ADC so disable battery reads

unsigned int avi_width, avi_height, avi_bpp; // AVI file size, colour depth
unsigned int avi_frametime, avi_framelen; //  AVI frame period and bytes per frame
unsigned int avi_frames, avi_framenum, avi_start; //AVI total frames, current frame, pointer to frame data in file
unsigned int battlevel; // battery voltage in mV
unsigned int tick;
unsigned int powerdowntimer;

unsigned char cardmounted, cardinsert;

// structures for use by filesystem

FSFILE * fptr; // filesystem file handle
SearchRec searchfile, searchdir;

// general purpose buffers 

// union to allow buffer access by bytes or words 

typedef union {
    unsigned char bytes[cambufsize];
    unsigned short shorts[cambufsize / 2];
    unsigned long words[cambufsize / 4];
} buffertype;

buffertype __attribute__((aligned(4))) buffer_union; // buffer for image data etc. union allows access as bytes, shorts or words



typedef union {
    unsigned char bytes[hbuflen];
    unsigned short shorts[hbuflen / 2];
    unsigned int words[hbuflen / 4];
} headerbuftype;

headerbuftype __attribute__((aligned(4))) headerbuf; // small buffer for AVI/BMP headers etc.