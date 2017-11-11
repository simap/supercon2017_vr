
#include "cambadge.h"
#include "globals.h"


#define maxpixsize 5
#define minpixsize 2

#define minparts 100
#define maxparts 1024 // these get divided by current pixel size


#define xmax (pxsize<<16) // limits of fixed-point coord values
#define ymax (pysize<<16)
#define normgravity 800
#define floatgravity 50

#define vmax 200000
#define yforce (-accy/8) // map accelerometer directions. scale to avoid overflow
#define xforce (accx/8)
#define stillthresh 200 // still-time after which fizzing starts
#define stillfizz 20 // rate of fizzing increase when still
#define maxfizz 4000 // max fizziness
#define movethresh 100 // moving-or-not threshold
#define minmass 64
#define maxmass 255

#define poke 100000
#define bounce 32 // fraction of reverse velocity on collision 256=100%
#define wiggle 8000 //absolute randomness of velocity on collision

typedef union {
    short sh[2];
    long lo;
} fixed; // 16:16 bit fixed-point data type

typedef struct { // struct of particle data
    fixed p_x;
    fixed p_y; // current position pixel:fraction
    int vx;
    int vy; // current velocity 
    int mass;
} parttype;

#define px p_x.lo // alias union members to simplify access
#define py p_y.lo
#define pixx p_x.sh[1]
#define pixy p_y.sh[1]




#define imagesize (dispwidth*dispheight)
#define pixbufsize (imagesize/(minpixsize*minpixsize))
#define imgbuf (cambuffer+pixbufsize)
#define pixbuf cambuffer // overlay pixel buffer with cambuffer memory ( replaces parttype p[maxparts]; 
// image buffer at cambuffer+pixbufsize
parttype *p = (parttype*) &cambuffer[pixbufsize + imagesize]; // overlay particle list with cambuffer memory ( replaces parttype p[maxparts]; 

#if (maxparts/minpixsize)*5*4+pixbufsize+imagesize>cambufsize 
#error too many particles
#endif



//________________________________________________________________________________________
unsigned int pxsize, pysize, pixsize;

unsigned int readpix(int x, int y) {
    unsigned int d;

    d = x + y*pxsize;
    if (d <= pxsize * pysize) return pixbuf[d];
    else return (-1);
}

char* particle(unsigned int action) {
    static int j, start = 0, nparts = 0, stilltimer, stillcount, lastx, lasty, gravity, edx, edy, editsize;
    static unsigned int framecnt;
    static unsigned char editmode; 

    int i, x, m, y, vx, vy, xx, yy, update, tweak;

    unsigned int c, d;
    static int masstable[8];
  

    if (action == act_name) return ("PARTICLE");
    else if (action == act_help) return ("Particle toy");

    if (action == act_start) {
        start = 1;
           
        printf(top butcol "EXIT             Edit" taby1 tabx17 "Mode" bot "Poke     #Parts  Size" );
        printf(del del cls);
    }

    if (action != act_poll) return (0);

    if (start) { // initialise
       
        editmode = 0;
        editsize = 1;
        framecnt = 0;
        srand(accx + accy + accz); // seed accelerometer
        mplotblock(0, 0, 128, 128, 0,imgbuf );//ensure borders clear from memory leftovers
        switch (start) {
            case 1:
                pixsize = minpixsize+1;
                nparts = randnum(minparts / pixsize, maxparts / pixsize); // fewer bigger pixels
                for (i = 0; i != pxsize * pysize; i++) pixbuf[i] = 0; //clear obstacles & pixels

                break;
            case 2: if (++pixsize > maxpixsize) pixsize = minpixsize;
                for (i = 0; i != pxsize * pysize; i++) pixbuf[i] = 0; //clear obstacles & pixels
                break;
            case 3:
                nparts = randnum(minparts / pixsize, maxparts / pixsize); // fewer bigger pixels
                break;

        }
        pxsize = 120 / pixsize; // use 120 as divisible by lots of things to avoid partial pixel issues
        pysize = 120 / pixsize;
        gravity = normgravity;

        for (i = 0; i != nparts; i++) {
            p[i].px = randnum(0, xmax - 1);
            p[i].py = randnum(0, ymax - 1);
            p[i].vx = 0; //(rand() % vmax)-vmax/2;
            p[i].vy = 0; //(rand() % vmax)-vmax/2;
            p[i].mass = randnum(1, 7);
        }

        for (i = 0; i != 8; masstable[i++] = (minmass + i * (maxmass - minmass) / 8) * maxpixsize / pixsize);
        for (i = 0; i != pxsize * pysize; i++) if (pixbuf[i] != 0xff) pixbuf[i] = 0; // clear pixel buffer, leave obstacles
        printf(cls whi "%3d Particles Size %1d" del, nparts, pixsize);
        start = 0;

    } // start

    if (!editmode) {
        if (butpress & powerbut) return ("");
        if (butpress & but3) start = 2;
        if (butpress & but2) start = 3;
        if (butpress & but1) for (i = 0; i != nparts; i++) {//poke
                p[i].vx = randnum(-poke, poke);
                p[i].vy = randnum(-poke, poke);
            }

    }

    if (butpress & but4) if (editmode ^= 1) { // toggle edit mode
            printf(cls butcol "Wipe            Quit" taby1 tabx11 "Edit mode" tabx0 taby11"Pen\n"  "Size      Clear   Set" del);

            edx = (pxsize / 2) << 8; // centre
            edy = (pysize / 2) << 8;
        }


    if (tick && !editmode) {
        i = (abs(xforce - lastx) + abs(yforce - lasty));
        if (i > movethresh) { // do fizzing
            stillcount = stillcount * 63 / 64;
            if (stillcount < stillthresh) stillcount = 0;
        } // reduce slowly to avoid cancelling from small move
        if (++stillcount > maxfizz) stillcount--;
        if (stillcount < stillthresh) {
            lastx = xforce;
            lasty = yforce;
        } // lock last value while fizzing for good recovery           
        //printf("X %d Y %d D %d C %d   NP %d\n",xforce,yforce,i,stillcount,nparts);

    } //tick 

    if (!editmode) for (i = 0; i != nparts; i++) {
            m = masstable[p[i].mass & 7];
            vx = p[i].vx + xforce * gravity * m / 65536;
            vy = p[i].vy + yforce * gravity * m / 65536;
            x = p[i].px + vx;
            y = p[i].py + vy;

            if (x < 0) x = 0;
            else if (x > xmax) x = xmax - 1;
            if (y < 0) y = 0;
            else if (y > ymax) y = ymax - 1;

            xx = x >> 16;
            yy = y >> 16;

            if ((readpix(xx, yy) != 0) && !((p[i].pixx == xx) && (p[i].pixy == yy))) { // something in the way (that's not me) - bounce and wiggle a bit

                p[i].vx = -(vx * bounce / 256) + randnum(-wiggle, wiggle);
                p[i].vy = -(vy * bounce / 256) + randnum(-wiggle, wiggle);
                if (stillcount > stillthresh) {// start fizzing if left for a while
                    p[i].vx = p[i].vx + randnum(-stillcount*stillfizz, stillcount * stillfizz);
                    p[i].vy = p[i].vy + randnum(-stillcount*stillfizz, stillcount * stillfizz);

                }


            } else { // nothing in the way - update positions and velocities
                p[i].px = x;
                p[i].py = y;
                p[i].vx = vx;
                p[i].vy = vy;
            }

            // clip speeds and positions
            if (p[i].vx > vmax) p[i].vx = vmax;
            else if (p[i].vx <= -vmax) p[i].vx = -vmax;
            if (p[i].vy > vmax) p[i].vy = vmax;
            else if (p[i].vy <= -vmax) p[i].vy = -vmax;

            if (p[i].px >= xmax) p[i].px = xmax - 1;
            if (p[i].px < 0) p[i].px = 0;
            if (p[i].py >= ymax) p[i].py = ymax - 1;
            if (p[i].py < 0) p[i].py = 0;

        } // recalc particles

    for (i = 0; i != pxsize * pysize; i++) if (pixbuf[i] != 0xff) pixbuf[i] = 0; // clear pixel buffer, leave obstacles

    for (i = 0; i != nparts; i++) { // plot new positions
        tweak = 1;
        do {
            d = p[i].pixx + p[i].pixy*pxsize; //index to pixel buffer

            if (pixbuf[d]) { // an obstacle has been put where we are, or there's a particle already there - move away
                p[i].pixx += randnum((p[i].pixx > tweak) ? -tweak : 0, (p[i].pixx < pxsize + tweak) ? tweak : 0); // move, keeping in range
                p[i].pixy += randnum((p[i].pixy > tweak) ? -tweak : 0, (p[i].pixy < pysize + tweak) ? tweak : 0);
                tweak++; // move further away each pass til we find an empty place       
            }
        } while (pixbuf[d]);

       if (d <= pxsize * pysize) pixbuf[d] = p[i].mass;
    }

    // draw new pixel buffer. 
     
 
    for (i = 0, yy = 0; yy != pysize; yy++)
        for (xx = 0; xx != pxsize; xx++)
            mplotblock(xx * pixsize + 4, yy * pixsize + 4, pixsize, pixsize, pixbuf[i++], imgbuf);

    // plot accelerometer bars
    i = 64 + accx / 256;
    j = 64;
    if (i > 64) {
        j = i;
        i = 64;
    }
    mplotblock(0, 126, 128, 2, 0, imgbuf);
    mplotblock(i, 126, j - i + 1, 2, 0xfe, imgbuf);

    i = 64 - accy / 256;
    j = 64;
    if (i > 64) {
        j = i;
        i = 64;
    }
    mplotblock(0, 0, 2, 128, 0,imgbuf );
    mplotblock(0, i, 2, j - i + 1, 0xfe, imgbuf);
    //fizz bar
    mplotblock(126, 0, 2, 128, 0, imgbuf);
    if (!editmode) {
        i = stillcount * 128 / (maxfizz - stillthresh);
        if (stillcount > stillthresh) mplotblock(126, 127 - i, 2, i, 1, imgbuf);
    }
#define edthr 256
    if (editmode) {
        if (tick) {
            i = (edx >> 8)+(edy >> 8) * pysize; //pixel buf offset of edit co-ords
            
            
            if (butpress & but1) editsize ^= 1;
            if (butstate & (but2|but3)) {
                d=((butstate&but3)?0xff:0); // set or clear
                  pixbuf[i] = d; // draw 
                if (editsize) {// draw 2x2
                    pixbuf[i + 1] = d;
                    pixbuf[i+ pxsize] = d;
                    pixbuf[i+ pxsize + 1] = d;
                } // big
            }//but2/3

            if (butstate & powerbut) for (i = 0; i != pxsize * pysize; i++) if (pixbuf[i] == 0xff) pixbuf[i] = 0; // clear obstacles

            if (abs(xforce) > edthr) edx += (xforce / 16); // edit coords fixed point with 8 bit fraction
            if (abs(yforce) > edthr) edy += (yforce / 16);
            if (edx < 0) edx = 0;
            if ((edx>>8) >= (pxsize - editsize )) edx = (pxsize - editsize - 1) << 8;
            if (edy < 0) edy = 0;
            if ((edy>>8) >= (pysize - editsize)) edy = (pysize - editsize - 1) << 8;
 
        }// edit tick        


        for (i = 0; i != 8; i++) palette[i] = primarycol[i + 8]; // dim particles
        palette[0xff] = c_whi; // obstacles
        //edit cursor 
        mplotblock((edx >> 8) * pixsize + 4, (edy >> 8) * pixsize + 4, pixsize * (editsize + 1), pixsize * (editsize + 1), (framecnt & 2) ? 0x80 : 0xff, imgbuf);



    } else {

        for (i = 0; i != 8; i++) palette[i] = primarycol[i];
        palette[0xff] = c_grey; // obstacles
    }
    palette[0xfe] = c_whi; // accel bars
   dispimage(0, 0, 128, 128, img_mono, imgbuf);
    ++framecnt;
    return (0);


}


