
#include "cambadge.h"
#include "globals.h"

// stuff to do with AVI,BMP formats etc.

unsigned long fgetword(unsigned long offset) { // get a word from offset in file
    unsigned long d;
    FSfseek(fptr, offset, SEEK_SET);
    FSfread(&d, 1, 4, fptr);
    return (d);
}

unsigned long mgetword(unsigned long offset) // return long at arbitary alignment from avi buffer
{
    // buffered version of fgetword reading from avibuf - avoids lots of slow seeks
    return (avibuf[offset] | ((unsigned long) avibuf[offset + 1] << 8) | ((unsigned long) avibuf[offset + 2] << 16) | ((unsigned long) avibuf[offset + 3] << 24));
}

unsigned int openavi(char* filename) { // open AVI and get parameters. returns <>0 if error
    unsigned int i, fileofs, chunknum, chunklen;
    avi_frametime = avi_frames = avi_width = avi_height = avi_bpp = 0; // ensure sensible values in case of error

    fptr = FSfopen(filename, FS_READ);
    if (fptr == NULL) return (1);
    if (FSfread(&avibuf, 0x100, 1, fptr) == 0) return (2); // read first block to RAM buffer to avoid lots of slow seeks and short reads
    if (mgetword(8) != 0x20495641) return (3); // not AVI

    fileofs = 0x0c;
    chunknum = mgetword(fileofs);
    chunklen = mgetword(fileofs + 4);
    if (chunknum != 0x5453494c) return (4); // Header LIST not found
    if (mgetword(fileofs + 8) != 0x6c726468) return (5); // hdr chunk not found
    avi_frametime = mgetword(fileofs + 0x14); // uS
    avi_frames = mgetword(fileofs + 0x24);
    avi_width = mgetword(fileofs + 0x34);
    avi_height = mgetword(fileofs + 0x38);
    do {
        fileofs = fileofs + chunklen + 8;
        chunknum = fgetword(fileofs);
        chunklen = fgetword(fileofs + 4);
    } while ((chunknum != 0x5453494c) && (!FSfeof(fptr))); //"LIST"
    if (chunknum != 0x5453494c) return (6); //Stream LIST not found

    if (fgetword(fileofs + 8) != 0x69766f6d) return (7); // "movi" chunk not found
    fileofs += 12;
    chunknum = fgetword(fileofs);
    if ((chunknum & 0xfeffffff) != 0x62643030) return (8); // 00db/dc not found
    avi_framelen = fgetword(fileofs + 4); // assume block length = frame size
    avi_bpp = avi_framelen / (avi_width * avi_height);
    avi_start = fileofs + 8; //->start of first frame
    avi_framenum = 0;
    if ((avi_framelen + 8) > cambufsize) return (9);
    if ((avi_bpp == 0) || (avi_bpp > 3)) return (10);
    if (avi_width > dispwidth) return (11);
    if (avi_height > dispheight) return (12);

    return (0);
}

unsigned int showavi(void) { // display 1 frame of AVI
    unsigned int i;

    if (++avi_framenum == avi_frames) {
        FSfseek(fptr, avi_start, SEEK_SET);
        avi_framenum = 0;
    }
    if (FSfread(&cambuffer, avi_framelen + 8, 1, fptr) == 0) return (2); // +8 to ship over chunk type/length - assume it's all video frames, no audio
    if(avi_bpp==1) monopalette(16,240); // mono AVIs use limited range
    // for some reason, 8 bit per pixel AVIs have reversed vertical scan
    dispimage((dispwidth - avi_width) / 2, (dispheight - avi_height) / 2, avi_width, avi_height, avi_bpp | ((avi_bpp > 1) ? img_revscan : 0), cambuffer);
    return (0);

}

unsigned int loadbmp(char *filename, unsigned int load) { // action = 0 for info, 1 for load, 2 for load & display
    unsigned int i;
    unsigned char c[4];
    fptr = FSfopen(filename, FS_READ);
    if (fptr == NULL) return (1);
    if (FSfread(&avibuf, 1, 14 + 40, fptr) == 0) {
        FSfclose(fptr);
        return (2);
    } // read header
    if ((mgetword(0) & 0xFFFF) != 0x4D42) {
        FSfclose(fptr);
        return (13);
    } // not BMP
    avi_start = mgetword(10); // -> start of image data
    avi_width = mgetword(14 + 4);
    avi_height = mgetword(14 + 8);
    i = mgetword(14 + 14) & 0xffff;
    avi_bpp = 0;
    if (i == 8) avi_bpp = 1;
    else if (i == 24) avi_bpp = 3;
    avi_framelen = avi_width * avi_height*avi_bpp;
    i = 0;
    if (avi_bpp == 0) i = 10;
    if (avi_width > dispwidth) i = 11;
    if (avi_height > dispheight) i = 12;
    if ((avi_framelen + 8) > cambufsize) i = 9;
    if (i) {
        FSfclose(fptr);
        return (i);
    }
    if (!load) {
        FSfclose(fptr);
        return (0);
    }

    if (avi_bpp == 1) {
        for (i = 0; i != 256; i++) { // assume full colour table, doesn't matter if not as garbage entries shouldn't be in image data 
            FSfread(&c, 1, 4, fptr);
            palette[i] = rgbto16(c[2], c[1], c[0]);

        }
    }

    FSfseek(fptr, avi_start, SEEK_SET);
    i = FSfread(&cambuffer, avi_framelen, 1, fptr);
    FSfclose(fptr);
    if (i != 1) return (2);

    if (load == 2) dispimage((dispwidth - avi_width) / 2, (dispheight - avi_height) / 2, avi_width, avi_height, (avi_bpp == 1) ? (img_mono |  img_revscan) : (img_rgb888 | img_revscan), cambuffer);
    return (0);

}

void aviword(unsigned int v, unsigned int offset) { // write word to AVI buffer
    avibuf[offset] = v;
    avibuf[offset + 1] = v >> 8;
    avibuf[offset + 2] = v >> 16;
    avibuf[offset + 3] = v >> 24;
}

unsigned int writebmpheader(unsigned int xsize, unsigned int ysize, unsigned int bpp) {//bpp is BYTES per pixel 1 or 3

    unsigned int i, j, k;
    for (i = 0; i != (40 + 14); avibuf[i++] = 0);

    avibuf[0] = 'B';
    avibuf[1] = 'M';
    i = 4 + 40 + xsize * ysize * bpp + ((bpp == 1) ? 1024 : 0); // size
    aviword(i, 2);
    avibuf[10] = 0x36;
    if (bpp == 1) avibuf[11] = 4; // 1024 bytes colour table
    avibuf[14] = 0x28; // hdr size
    aviword(xsize, 18); //width
    aviword(ysize, 22); //height

    avibuf[26] = 1; //planes
    avibuf[28] = bpp * 8; // bytes/pixel

    if (bpp == 1) avibuf[47] = 1; // colours used in pallette

    i = (FSfwrite(&avibuf[0], 40 + 14, 1, fptr));
    if (bpp == 3) return (i);


    // write pallette in 128 byte chunks due to size of avibuf
    for (j = 0, i = 0; i != 256; i++) {

        avibuf[0] = i;
        avibuf[1] = i;
        avibuf[2] = i;
        avibuf[3] = 0;
        k = FSfwrite(&avibuf[0], 0x4, 1, fptr);
        if (k == 0)i = 256;
    }

    return (k);

}

void conv16_24(unsigned int npixels, unsigned int offset)
//convert 16bpp RGB565 to 8BPP RGB in cambuffer +1 
{
    unsigned int i, sp, dp, r, g, b;
    unsigned char *buf;
    buf = cambuffer + offset;

    sp = npixels * 2;
    dp = npixels * 3;

    do {
        i = (unsigned int) buf[--sp] << 8;
        i |= buf[--sp];
        r = (i & 0x1f) << 3;
        g = (i & 0x7E0) >> 3;
        b = (i >> 8)&0xF8;
        buf[--dp] = b;
        buf[--dp] = g;
        buf[--dp] = r;
    } while (sp);
}

void flipcambuf(unsigned int xpixels, unsigned int ypixels, unsigned int offset) { // yflip camera buffer and restrict range of mono AVI
    unsigned int i, d, x, y, sp, dp;
    unsigned char *buf;
    for(i=0;i!=256;i++) avibuf[i]=16+i*223/255; // lookup table for 0-255 range to 16-223
    buf = cambuffer + offset;
    for (y = 0; y != ypixels / 2; y++) {
        sp = y*xpixels;

        dp = (ypixels - y - 1) * xpixels;

        for (x = 0; x != xpixels; x++) {
            d = buf[sp];
            buf[sp++] = avibuf[buf[dp]];
            buf[dp++] = avibuf[d];
        }
    }

}

unsigned int startavi(void) { // reserve space for AVI header
    return (FSfwrite(&avibuf[0], (avi_bpp == 2) ? 0xEC : 0xE0, 1, fptr) == 0);
}

unsigned int finishavi(void) {// write index table and fill in AVI header
    unsigned int i;
    for (i = 0; i != 0x100; avibuf[i++] = 0);

    // write index first as we're already at right place in file
    aviword(0x31786469, 0); // idx1  
    aviword(avi_frames * 0x10, 4); // length

    if (FSfwrite(&avibuf[0], 8, 1, fptr) == 0) return (1);

    aviword(0x63643030, 0); // 00dc
    aviword(0x10, 4); // length
    aviword(avi_framelen, 12); // length

    for (i = 0; i != avi_frames; i++) {
        aviword((avi_framelen + 8) * i + 4, 8);

        if (FSfwrite(&avibuf[0], 16, 1, fptr) == 0) return (2);

    }

    if (FSfseek(fptr, 0, SEEK_SET)) return (3); // go to start of file to do header

    aviword(0x46464952, 0); // RIFF
    // +4 = total file length-8  - calc from framesize, bpp and #frames
    aviword(avi_frames * (avi_framelen + 0x18)+((avi_bpp == 2) ? 0xec : 0xe0), 0x04);

    aviword(0x20495641, 8); // AVI 
    aviword(0x5453494c, 0x0c); //LIST
    aviword((avi_bpp == 2) ? 0xCC : 0xC0, 0x10); // length of all header sections

    aviword(0x6c726468, 0x14); // hdrl
    aviword(0x68697661, 0x18); //avih
    aviword(0x38, 0x1c); // avi header length
    if (avi_frametime == 0) avi_frametime = 10; // avoid divby0 if not set up
    aviword(avi_frametime, 0x20); // framerate

    aviword(avi_framelen * 1000000 / avi_frametime, 0x24); // max bytes/sec
    //40 padding granularity
    aviword(0x10, 0x2c); // flags has_index
    aviword(avi_frames, 0x30);

    //52 unused
    aviword(1, 0x38); // streams
    //60 bufsize
    aviword(avi_width, 0x40);
    aviword(avi_height, 0x44);
    //72,76,80,84 unused
    aviword(0x5453494c, 0x58); //LIST
    aviword((avi_bpp == 2) ? 0x80 : 0x74, 0x5c); //92 length of LIST - TBD
    aviword(0x6c727473, 0x60); // strl
    aviword(0x68727473, 0x64); // strh8
    aviword(0x38, 0x68); // length of vids
    aviword(0x73646976, 0x6c); // vids
    if (avi_bpp == 1) aviword(0x20203859, 0x70); // stream type Y8
    else aviword(0x73646976, 0x70); // stream type vids



    //112,116,120 unused flags,priority/language
    aviword(1, 0x80); //scale
    aviword(1000000 / avi_frametime, 0x84); // rate

    aviword(avi_frames, 0x8c); //#frames - fill later
    aviword(avi_framelen, 0x90);
    aviword(0xffffffff, 0x94); // quality, copied from sample file
    //160,164 unused
    aviword(avi_width, 0xa0);
    aviword(avi_height, 0xa2);

    aviword(0x66727473, 0xa4); // strf
    aviword((avi_bpp == 2) ? 0x34 : 0x28, 0xa8); //length

    // bitmap header
    aviword(0x28, 0xac); //length
    aviword(avi_width, 0xb0);
    aviword(avi_height, 0xb4);
    aviword(1, 0xb8); //planes
    aviword(avi_bpp * 8, 0xba); //bpp
    //208 compression
    if (avi_bpp == 1) aviword(0x20203859, 0xbc); // Y8
    if (avi_bpp == 2) aviword(3, 0xbc); // rgb565

    aviword(avi_framelen, 0xc0);
    i = 0xd4;
    if (avi_bpp == 2) {// colour bit masks for RGB565 ( from looking at virtualdub output)
        aviword(0x0000f800, 0xd4);
        aviword(0x000007E0, 0xd8);
        aviword(0x0000001F, 0xdc);
        i += 12;
    }

    aviword(0x5453494C, i);
    i += 4; //LIST
    aviword((avi_framelen + 8) * avi_frames + 4, i);
    i += 4;
    aviword(0x69766f6d, i);
    i += 4; // movi

    if (FSfwrite(&avibuf[0], i, 1, fptr) == 0) return (4);

    FSfclose(fptr);
    return (0);
}
