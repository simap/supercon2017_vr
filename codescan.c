
#include "cambadge.h"
#include "globals.h"
 
//code 128 barcode reader
// Mike Harrison 15 Oct 2017
// reads Code 128 barcodes. Codes need to be approx 1 inch per symbol wide
// Assumes alpha (prefix B) only - optimisations for numerics not currently supported
#define debugcode 0 // 1= to send debug to uart1, =2 for uart2
// table of code-128 symbols, 2 bits per symbol, 0 = length 4, ls first

#define debuglist 0 // <>0 to output pixel line data and show baseline etc. to optimise slicing
 
const unsigned short codetable[108] = {
    0x9AA, 0xA9A, 0xAA9, 0x66B, 0x67A, 0x76A, 0x6A7, 0x6B6, 0x7A6, 0xA67, 0xA76, 0xB66, 0x5AE, 0x69E, 0x6AD, 0x5EA, 0x6DA,
    0x6E9, 0xAE5, 0xA5E, 0xA6D, 0x9E6, 0xAD6, 0xD9D, 0xD6A, 0xE5A, 0xE69, 0xDA6, 0xE96, 0xEA5, 0x99B, 0x9B9, 0xB99, 0x57B,
    0x75B, 0x779, 0x5B7, 0x797, 0x7B5, 0x977, 0xB57, 0xB75, 0x59F, 0x5BD, 0x79D, 0x5DB, 0x5F9, 0x7D9, 0xDD9, 0x97D, 0xB5D,
    0x9D7, 0x9F5, 0x9DD, 0xD5B, 0xD79, 0xF59, 0xD97, 0xDB5, 0xF95, 0xD15, 0xA45, 0x355, 0x568, 0x54A, 0x658, 0x649, 0x45A,
    0x469, 0x5A4, 0x586, 0x694, 0x685, 0x496, 0x4A5, 0x865, 0xA54, 0x1D5, 0x856, 0x715, 0x562, 0x652, 0x661, 0x526, 0x616,
    0x625, 0x166, 0x256, 0x265, 0x991, 0x919, 0x199, 0x553, 0x571, 0x751, 0x517, 0x535, 0x157, 0x175, 0x5D1, 0x51D, 0xD51,
    0x15D, 0x946, 0x964, 0x96E, 0xBD5, 0x95F
};

static barstart, barlen, bardir, hyst;


// states used by this application

#define s_start 0
#define s_restart 1
#define s_run 2
#define lenbuf headerbuf.shorts
#define lenbuflen sizeof(headerbuf)/2

unsigned char lookupcode(unsigned int code) {
    // search for code, return value 0-106, FF if not found 
    // currently does linear search - could be optimised by sorting the table & doing binary search, 
    // but we only get here once we have correct-length symbols, so not worth the effort ( and would use extra storage for symbol number in sorted table) 

    unsigned int i, c;

    c = 0xff;
    i = 0;
    do if (codetable[i] == code) c = i;
        while ((i++ < sizeof (codetable)) && (c == 0xff));

    return (c);

}

#define maxcodelen 16
static unsigned char barcode[maxcodelen + 1];

unsigned char* trycode(unsigned int bits) {
    // attempt a read from lengths in buffer. 

    unsigned int i, t, w, x, bt1, bt2, bt3, bitcnt, len, code, digit, gotstop, table, startpix, gotstart;
    unsigned short codebuf[maxcodelen + 1];
    unsigned char* codeptr;

    barlen = 0;

#if debugcode!=0
    printf("\nLengths: ");
    for (i = 0; i != bits - 2; i++) {
        printf("%02d ", lenbuf[i + 2]);
        if ((i % 6) == 5) printf(" ");
    }
    printf("\n");
#endif     
    startpix = 2; // first 2 will always be garbage at start of line

    gotstart = 0;
    digit = 0;
    gotstop = 0;

    do {

        if (gotstart == 0) {
#if debugcode!=0
            printf("\nStart %02d ", startpix);
#endif                 
            x = startpix;

            startpix += 2;
            if (startpix > xpixels / 4) return (0); // give up if start not found in first quarter

            // all start codes have 6 transitions, so assume we have a good start code and derive bar-length thresholds from that   
            len = (unsigned int) lenbuf[x] + lenbuf[x + 1] + lenbuf[x + 2] + lenbuf[x + 3] + lenbuf[x + 4] + lenbuf[x + 5];

            bt1 = len * 3 / 22; // bar/gap < this is a 1
            bt2 = len * 5 / 22; // bar/gap < this is a 2
            bt3 = len * 7 / 22; // bar/gap < this is a 3, >= is a 4       

#if debugcode!=0
            printf("Thrs %d %d %d ", bt1, bt2, bt3);
#endif 

        }// gotstart=0


        len = 0; //length so far in pixels
        bitcnt = 0; // number of bar time cells so far
        code = 0; // shift register for symbol code
        do { // get a symbol
            w = lenbuf[x++]; // width of current bar or space
            len += w; // total length so far
            t = 1; // code type
            if (w >= bt3) t = 4; //  0 = 4
            else if (w >= bt2) t = 3;
            else if (w >= bt1) t = 2;
            bitcnt += t; // number of units in current symbol
            code = code << 2 | (t & 3);
            if (x >= bits) return (0); // ran out of bits
        } while (bitcnt < 11);

        if (gotstart == 0) {
            if (bitcnt == 11) if (((code == 0x946) || (code == 0x964) || (code == 0x96e))) gotstart = 1;
        }

#if debugcode !=0
        t = code;
        for (i = 0; i != 6; i++) printf("%d", (t >> ((5 - i)*2)&3));
        printf(" Gstart %1d code %4X  Sym bits %2d tot len %3d ", gotstart, code, bitcnt, len);
#endif            
        if (gotstart) {
            barlen += len;
            if (bitcnt != 11) return (0); // bad count  fatal now
            codebuf[digit++] = code;
            gotstop = (code == codetable[106]); // is it the stop code ? 

            bt1 = len * 3 / 22; // update length thresholds from length of last 11-bit cell to track camera distortions etc. 
            bt2 = len * 5 / 22; // this makes a BIG difference to reliability with crappy cheap lens!
            bt3 = len * 7 / 22;

#if debugcode !=0
            printf(" %c  new Thrs %3d %3d %3d\n", lookupcode(code) + ' ', bt1, bt2, bt3);
#endif

        }// got start


    } while (!gotstop);
#if debugcode !=0
    printf("\n\n");
#endif           
    if (!gotstop) return (0); // ran out of bits before seeing stop code

    switch (codebuf[0]) { // which start code
        case 0x946: table = 0;
            break;
        case 0x964: table = 1;
            break;
        case 0x96e: table = 2;
            break;
        default: table = 3;
            break;

    }
#if debugcode !=0
    printf("Got %d symbols, table = %d stopvalid=%d\n", digit, table, codebuf[digit - 1] == codetable[106]);

#endif     
    if (table == 3) return (0); // not got a valid start code
    if (codebuf[digit - 1] != codetable[106]) return (0); // bad stop code

    for (i = 0; i != digit; i++)
        if ((codebuf[i] = lookupcode(codebuf[i])) == 0xff) return (0); // exit if any bad code found 
    // calculate check digit
    x = codebuf[0];
    for (i = 1; i != digit - 2; i++) x += codebuf[i] * i;
    x %= 103;
#if debugcode !=0 
    printf("Calculated check = %3d, Read check = %3d\n", x, codebuf[i]);
#endif
    if (x != codebuf[i]) return (0);
    // assume table 2 for the moment and no table-change codes
    codeptr = barcode;
    for (i = 0; i != digit - 3; i++) *codeptr++ = codebuf[i + 1] + ' ';

    for (barstart = 0, i = 0; i != startpix - 2; barstart += (unsigned int) lenbuf[i++]);

    barlen += bt3; //;length

    return (barcode);
}

unsigned char* readcode(unsigned int bufoffset, unsigned int xpixels) {
    unsigned int i, x, c, black, len, bits, base, dir, thresh;
    unsigned char* result;
    for (i = 0; i != maxcodelen + 1; barcode[i++] = 0);

    // read bar code in 1 line of greyscale image in cambuffer[offset], length xpixels
    // returns 0 for fail or pointer to ASCII result. 
    // barstart,barlen indicate start pixel/npixels, bardir = direction of read

    // digitise pixel line into list of lengths, try reading in both directions
    // this could be improved to better measure lengths from centre of each bar
    // presently just looks for level to have changed  by >thresh from where the last peak 

    thresh = 20; // seems a reasonable compromise
    dir = 0; // scan direction 
    do {

        black = 1; //assume starting at black
        base = 0;
        len = 0;
        bits = 0;

        for (x = 0; x != xpixels; x++) {
            if (dir) c = cambuffer[bufoffset + xpixels - 1 - x];
            else c = cambuffer[bufoffset + x]; // scan forwards or backwards

#if debugcode !=0 
            if (x < 200) printf("%3d ", c);
#endif
            if (black) { // currently black
                if (c > base + thresh) {// edge found

#if debugcode !=0 
                    if (x < 200) printf("\nB");
#endif
                    base = c;
                    black = 0;
                    lenbuf[bits++] = len; // length of black
                    len = 0;
                }
                if (c < base) base = c; // track peak

            } else { // currently white
                if (c < base - thresh) {
#if debugcode !=0 
                    if (x < 200) printf("\nW");
#endif
                    base = c;
                    black = 1;
                    lenbuf[bits++] = len; // length of white
                    len = 0;
                }
                if (c > base) base = c; // track peak
            }//white->black
            len += 4; //+4 per pixel to allow space for rounding when calculating threshold         
            if (bits >= lenbuflen) return (0);

#if debuglist!=0
            if (butstate & but5) {
                dispuart = debuglist;
                printf("%d,%d,%d,%d,%d\n", x, black * 256, c, base, thresh);
                if (x == xpixels - 1) printf("--------------\n");
                dispuart = 0;
            }
#endif           

        }


        dispuart = 0;


#if debugcode==0
        if (bits < 24) return (0); // can't be enough data (start+code+check+stop)
#endif 
        // now we have a list of lengths . black lengths will be at even addresses, white on odd

#if debugcode !=0 

        dir = 1;

#endif


        result = trycode(bits);

        bardir = dir;

    } while ((result == 0) & (++dir != 2));

    if (result == 0) return (0);

    barstart /= 4;
    barlen /= 4;
    if (bardir) barstart = xpixels - barstart - barlen - 1;

    return (result);


}




//==================================================================================================

char* codescan(unsigned int action) {
    static unsigned int state, hires, explock,scanlines;
    unsigned int i, x, y, min, max, c, a, s, t, d, w, l, bits, base;
    char* thecode;
#define height 32 // 1280 bytes/line so can only do limited height
#define ypos 16

    switch (action) {
        case act_name: return ("CODESCAN");
        case act_help: return ("CODE128 Barcode\nscanner");
        case act_init:
            // add any code here that needs to run once at powerup - e.g. hardware detection/initialisation      
            return (0);

        case act_powerdown:
            // add any code here that needs to run before powerdown
            return (0);

        case act_start:
            // called once when app is selected from menu
            hires = 0;
            scanlines=1;
            explock = 0;
            state = s_start;
            return (0);
    } //switch

    if (action != act_poll) return (0);

    if (!tick) return (0);


    switch (state) {
        case s_start:
            hyst = 20;

            cam_enable(cammode_128x96_z4_mono);
            xstart = 0;


            ypixels = height;

            if (hires) {
                ystart = 30;
                ydiv = 27;
                cam_setreg(0x12, 0x00); // sxga
                xpixels = 1280;
            }
            else {
                ystart = 10;
                ydiv = 15;
                cam_setreg(0x12, 0x40); // VGA  mode
                xpixels = 640;
            }
            cam_grabenable(camen_grab, 7, 0);
            printf(cls);
        case s_restart:
            printf(top butcol "EXIT  " inv "CODESCAN" inv "  Light" bot);
            if (hires) printf(butcol inv "HI-RES" inv);
            else printf("HI-RES");

            if (explock) printf("  " inv "ExLock" inv);
            else printf("  ExLock");

            printf( "  Lines");
            
            state = s_run;
            break;

        case s_run:
            if (butpress & powerbut) {
                cam_enable(0);
                return ("");
            }

            if (butpress & but1) {
                hires ^= 1;
                state = s_start;
                break;
            }

            if (butpress & but2) {
                explock ^= 1;
                cam_setreg(0x13, explock ? 0xe0 : 0xe7);
                state = s_restart;
            }
            if (butpress & but3) {
                if(++scanlines==5) scanlines=1;

            }


            if (butpress & but4) if (led1) led1_on;
                else led1_off;


            if (!cam_newframe) break;


            monopalette(0, 255);
            //downscale 640 or 1280 to screen width
            dispimage(0, ypos, 128, height, img_mono | img_revscan | img_vdouble | (hires ? img_skip9 : img_skip4), cambuffer + 8);

            for(i=1;i!=scanlines+1;i++)  plotblock(4,ypos+height*2*i/(scanlines+1),120,1,c_red);
    
            for(i=1;i!=scanlines+1;i++) {
            t=((scanlines+1)-i)*height/(scanlines+1);  // round to line 
            
           // y = height * (hires ? 1280 : 640) *t + 8; // where in image to sample
            y=t*(hires ? 1280 : 640)+8;
            dispuart = debugcode;

            thecode = readcode(y, hires ? 1280 : 640);

#if debugcode !=0     
            printf("\n");
            dispuart = 0;
#endif

            if (thecode == 0) printf(tabx0 whi "%c                ",0xA7+i);
            else {
                printf(tabx0  "%c%10s",0xA7+i, thecode);
                plotblock(barstart / (hires ? 10 : 5), ypos+height*2*i/(scanlines+1), barlen / (hires ? 10 : 5), 2, bardir ? c_yel : c_grn);
            }
            }// for lines

#if debugcode !=0     
            delayus(500000);
#endif
            cam_grabenable(camen_grab, 7, 0);
            break;






    } // switch state





    return (0);

}


