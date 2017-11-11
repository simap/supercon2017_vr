#include "cambadge.h"
#include "globals.h"
#include "font6x8.inc"
#include "monorgb.h" // mono8->rgb565 lookup

// oled & display formatting stuff

void oledcmd(unsigned int d) { // send byte to display, bit 8 set for command, clear for data
    while (SPI1STATbits.SPIBUSY); // in case previous buffered data still being sent
    if (d & 0x100) oled_cd_lo;
    else oled_cd_hi;
    oled_cs_lo;
    SPI1BUF = d;
    while (SPI1STATbits.SPIBUSY);
    oled_cs_hi;
}

void oled_init(void) { //initialise display after powerup. see SSD1351 chip and OLED module datasheet for details
    unsigned int i;
    const unsigned short oledinitdata[] = {
        0x1fd, 0x12,
        0x1fd, 0xb1,
        0x1ae,
        0x1b3, 0xf1,
        0x1ca, 0x7f,
        0x1a2, 0x00,
        0x1a1, 0x00,
#if oled_upscan==1
        0x1a0, 0x35, // colour depth b0 rotate, b1 mirror b2 color orded, b4 v flip, b7,7 colour depth
#else  
        0x1a0, 0x37, // colour depth b0 rotate, b1 mirror b2 color orded, b4 v flip, b7,7 colour depth
#endif
        0x1b5, 0x0C, // GPIO gpio1 backlight power, gpio0 camera powerdown
        0x1ab, 0x01, //function sel
        0x1b4, 0xa0, 0xb5, 0x55, // seg low voltage
        0x1c1, 0xc8, 0x80, 0xc8, // contrast current
        0x1c7, 0x0f, // master current
        0x1b8, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x15, 0x17, 0x19, 0x1b, 0x1d, 0x1f,
        0x21, 0x23, 0x25, 0x27, 0x2a, 0x2d, 0x30, 0x33, 0x36, 0x39, 0x3c, 0x3f, 0x42, 0x45, 0x48, 0x4c, 0x50, 0x54, 0x58, 0x5c, 0x60, 0x64, 0x68, 0x6c,
        0x70, 0x74, 0x78, 0x7d, 0x82, 0x87, 0x8c, 0x91, 0x96, 0x9b, 0xa0, 0xa5, 0xaa, 0xaf, 0xb4,
        0xb1, 0x32, 0xb2, 0xa4, 0x00, 0x00, 0xbb, 0x17, 0xb6, 0x01, 0xbe, 0x05,
        0x1a6, // display mode
        0x115, 0x00, 0x7f, // col
        0x175, 0x00, 0x7f, // row
        0x1af
    };

    oled_rst_hi;
    delayus(oled_reset_time);
    oled_rst_lo;
    delayus(oled_reset_time);
    oled_rst_hi;
    delayus(oled_reset_time);
    
    for (i = 0; i != sizeof (oledinitdata) / 2; oledcmd(oledinitdata[i++]));
     while (SPI1STATbits.SPIBUSY);    oled_cs_hi;
    plotblock(0, 0, dispwidth, dispheight, 0); // clear display memory
    dispx = dispy = 0;
    fgcol = 0xffff;
    bgcol = 0;
   
}

void monopalette(unsigned int min, unsigned int max) {
    unsigned int i, d;
    for (i = 0; i != 256; i++) {
        d = (i - min)*255 / (max - min);
        if (i <= min) d = 0;
        if (i >= max) d = 255;
        palette[i] = rgbto16(d, d, d);
    }
}




void plotblock(unsigned int xstart, unsigned int ystart, unsigned int xsize, unsigned int ysize, unsigned int col)
{
    unsigned int i;

    
 if ((xstart + xsize > dispwidth) || (ystart + ysize > dispheight) || (xsize == 0) || (ysize == 0)) return; 
    
        oled_cs_lo;
        oled_cd_lo;
        SPI1BUF=0x75; while (SPI1STATbits.SPIBUSY); oled_cd_hi;
        SPI1BUF=xstart; 
        SPI1BUF=xstart + xsize - 1; 
        while (SPI1STATbits.SPIBUSY);
    

#if oled_upscan==1
       oled_cd_lo;      
        SPI1BUF=0x15; 
        i=((dispheight - ystart - 1) - ysize + 1); 
        while (SPI1STATbits.SPIBUSY); 
        oled_cd_hi;
        SPI1BUF=i; 
        SPI1BUF=dispheight - ystart - 1;
        while (SPI1STATbits.SPIBUSY); 

#else
        
        oled_cd_lo;      
        SPI1BUF=0x15; 
       while (SPI1STATbits.SPIBUSY); 
        oled_cd_hi;
        SPI1BUF=ystart; 
        SPI1BUF=ystart + ysize - 1;
        while (SPI1STATbits.SPIBUSY); 

#endif  
    
 
oled_cd_lo;      
        SPI1BUF=0x15c;   //send data
        i = xsize*ysize;
       while (SPI1STATbits.SPIBUSY); 
        oled_cd_hi;
     SPI1CONbits.MODE16 = 1; // 16 bit for ease of accesss - no speed improvement

    do {
        while (SPI1STATbits.SPITBF);
        SPI1BUF = col;
        
    } while(--i);
      while (SPI1STATbits.SPIBUSY); // wait until last byte sent before releasing CS. if we were DMAing, this would be done in DMA complete int
    SPI1CONbits.MODE16 = 0; // back to 8 bit mode
    oled_cs_hi;
}


void mplotblock(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int colour, unsigned char* imgaddr) {
    // plot block in memory buffer for subsequent display using dispimage, 8bpp only.
    // speeds up displays of lots of pixels.
    
    unsigned int xx, yy, gap;
  
    gap = dispwidth - width;
    imgaddr += (y * dispwidth + x);
    
    if(((x+width)>dispwidth) || ((y+height)>dispheight)) return;
    
    for (yy = 0; yy != height; yy++) {
        for (xx = 0; xx != width; xx++) *imgaddr++ = (unsigned char) colour;
        imgaddr += gap;
    }
   

}

void dispimage(unsigned int xstart, unsigned int ystart, unsigned int xsize, unsigned int ysize, unsigned int format, unsigned char* imgaddr) { // display image or solid colour in various formats. Note assumes format = bytes per pixel
    // for solid, image pointer is solid colour value, called via plotblock macro
    unsigned int i, d, e, y, x, r, g, b, bpp, skip,vdup,vcount;
    unsigned char *imgaddr2;
    bpp = format & 3;
    skip = (format & 0xf0) >> 4;
    vdup=(format & img_vdouble)?2:1;
   
#if oled_upscan==1
    format ^= img_revscan;
#endif
    if ((xstart + xsize > dispwidth) || (ystart + ysize > dispheight) || (xsize == 0) || (ysize == 0)) return;
    
    oledcmd(0x175);
    oledcmd(xstart);
    oledcmd(xstart + xsize - 1); // column address
#if oled_upscan==1
    oledcmd(0x115);
    oledcmd((dispheight - ystart - 1) - ysize*vdup + 1);
    oledcmd(dispheight - ystart - 1); // row address
#else   
    oledcmd(0x115);
    oledcmd(ystart);
    oledcmd(ystart + ysize*vdup - 1); // row address
#endif 

    oledcmd(0x15c); //send data

    i = xsize*ysize;
    if (((unsigned int) imgaddr + i * bpp * (skip + 1)) >= ((unsigned int) &cambuffer + cambufsize)) return;

    oled_cd_hi;
    oled_cs_lo;
    SPI1CONbits.MODE16 = 1; // 16 bit for ease of accesss - no speed improvement

    //contrary to what datasheet implies, it doesn't seem necessary to de-assert CS between bytes, 
    // so we can use buffered mode to avoid gaps between bytes for higher throughput.

    // Code doesn't need to be super-efficient as speed is limited by OLED max SPI clock rate. 
    // as long as we can produce 1 pixel every 1.3uS avaraged over the 8 pixel FIFO size we're maxing out the SPI bus


    for (y = 0; y != ysize; y++) {
        
        for(vcount=0;vcount!=vdup;vcount++) {
            
        if (format & img_revscan) imgaddr2 = imgaddr + (ysize - y - 1) * xsize * bpp *(skip + 1);
        else imgaddr2 = imgaddr + y * xsize * bpp * (skip + 1);

        for (x = 0; x != xsize; x++) {
            switch (bpp) {
                case 0: d = (unsigned int) imgaddr;
                    break;

                case 1: d = palette[*imgaddr2++];
                    imgaddr2 += skip;
                    break;
                case 2:
                    d = *imgaddr2++;
                    d |= (*imgaddr2++) << 8;
                    imgaddr2 += skip * 2;
                    break;

                case 3:
                    b = *imgaddr2++;
                    g = *imgaddr2++;
                    r = *imgaddr2++;
                    d = (r << 8 & 0xf800) | (g << 3 & 0x7C0) | (b >> 3);
                    imgaddr2 += skip * 3;
            } // switch bpp   

            while (SPI1STATbits.SPITBF);
            SPI1BUF = d;
        } // for x
        } // for vcount
    }// for y 


    while (SPI1STATbits.SPIBUSY); // wait until last byte sent before releasing CS. if we were DMAing, this would be done in DMA complete int
    SPI1CONbits.MODE16 = 0; // back to 8 bit mode
    oled_cs_hi;

}

void _mon_putc(char c) // STDIO for printf
{
    dispchar(c);
}

void dispchar(unsigned char c) {// display 1 character, do control characters
    unsigned int x, y, b, m;

    if (dispuart) {
      
        if (dispuart & dispuart_u1) u1txbyte(c);
        else u2txbyte(c); //UART mode
        if(!(dispuart & dispuart_screen) ) return;
    }

    switch (c) { // control characters

        case 2: //0.5s delay
            delayus(500000);
            break;
        case 3: // half space
            dispx += charwidth / 2;
            break;
        case 4: // short backspace
            dispx -=3;
            break;
        case 7:
            x = fgcol;
            fgcol = bgcol;
            bgcol = x;
            break; // invert  

        case 8:// BS 
            if (dispx >= charwidth) dispx -= charwidth;
            break;
        case 10:// crlf 
            dispx = 0;
            dispy += vspace;
            if (dispy >= dispheight) dispy = 0;
            break;
        case 12: // CLS 
            plotblock(0, 0, dispwidth, dispheight, bgcol);
            dispx = dispy = 0;
            break;
        case 13:
            dispx = 0;
            break; // CR
        case 14 : // grey
            fgcol=rgbto16(128,128,128);
            break;

        case 0x80 ... 0x93: // tab x
            dispx = (c & 0x1f) * charwidth;
            break;
        case 0xa0 ... 0xaf: // tab y
            dispy = (c & 0x0f) * vspace;
            break;

        case 0xc0 ... 0xff: // set text primary colour 0b11rgbRGB bg FG
            fgcol = rgbto16((c & 1) ? 255 : 0, (c & 2) ? 255 : 0, (c & 4) ? 255 : 0);
            bgcol = rgbto16((c & 8) ? 255 : 0, (c & 16) ? 255 : 0, (c & 32) ? 255 : 0);
            break;

        case startchar ... (nchars_6x8 + startchar - 1): // displayed characters
            oled_cs_lo;
            oledcmd(0x175);
            oledcmd(dispx);
            oledcmd(dispx + charwidth - 1); // column address
#if oled_upscan==1
            oledcmd(0x115);
            oledcmd((dispy + charheight - 1)^127);
            oledcmd(dispy^127); // row address

#else
            oledcmd(0x115);
            oledcmd(dispy);
            oledcmd(dispy + charheight - 1); // row address
#endif
            oledcmd(0x15c); //send data  
            SPI1CONbits.MODE16 = 1; // 16 bit SPI so 1 transfer per pixel
            oled_cd_hi;
            oled_cs_lo;
            c -= startchar;
            for (y = 0; y != charheight; y++) {
#if oled_upscan==1
                b = FONT6x8[c][7 - y]; //lookup outside loop for speed    
#else
                b = FONT6x8[c][y]; //lookup outside loop for speed  
#endif          
                for (x = 0; x != charwidth; x++) {
                    while (SPI1STATbits.SPITBF);
                    SPI1BUF = (b & 0x80) ? fgcol : bgcol;
                    b <<= 1;
                }
            } //y
            while (SPI1STATbits.SPIBUSY); // wait until last byte sent before releasing CS
            SPI1CONbits.MODE16 = 0;
            oled_cs_hi;
            dispx += charwidth;
            if (dispx >= dispwidth) {
                dispx = 0;
                dispy += vspace;
                if (dispy >= dispheight) dispy = 0;
            }
            break;

    }//switch

  oled_cs_hi;

}
