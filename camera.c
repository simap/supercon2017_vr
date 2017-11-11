#include "cambadge.h"
#include "globals.h"
// camera application

#define s_camstart 0
#define s_camlive 1
#define s_camsave 2
#define s_camsaved 3
#define s_camerror 4
#define s_camgrab 5
#define s_camwait 6
#define s_camrestart 7
#define s_camquit  8
#define s_camavistart 9
#define s_waitavi 10
#define s_aviloop 11
#define s_avierr 12


#define ct_bmp 0
#define ct_dir 1
#define ct_avi 2


char camname[12];

void docamname(unsigned int n, unsigned int ct) { // create a formatted filename CAMxxx.BMP or AVI, or dirname CAM000 

    unsigned int i = 0;
    if (ct == ct_dir) {
        camname[i++] = '\\';
    }
    camname[i++] = 'C';
    camname[i++] = 'A';
    camname[i++] = 'M';
    camname[i++] = (n / 1000) + '0';
    camname[i++] = ((n % 1000) / 100) + '0';
    camname[i++] = ((n % 100) / 10) + '0';
    camname[i++] = (n % 10) + '0';

    if (ct == ct_bmp) {
        camname[i++] = '.';
        camname[i++] = 'B';
        camname[i++] = 'M';
        camname[i++] = 'P';
    }
    if (ct == ct_avi) {
        camname[i++] = '.';
        camname[i++] = 'A';
        camname[i++] = 'V';
        camname[i++] = 'I';
    }

    camname[i++] = 0;
}

char* camera(unsigned int action) {
    static unsigned int camstate = s_camstart;
    static unsigned int camfile = 0, camdir = 0, cam_cammode, vidmode = 0, frame;
    static unsigned int rectime, explock,campage;
    unsigned int i;


    if (action == act_name) return ("CAMERA");
    else if (action == act_help) return ("Takes pictures");
    if (action == act_start) camstate = s_camstart;
    if (action != act_poll) return (0);
    switch (camstate) {

        case s_camstart:
            explock = 0;
            vidmode = 0;
            camfile = 0;
            camdir = 0;
            camdir = 0;
            cammode = cammode_128x96_z1;
            cam_enable(cammode); // buffer offset  so camera data is word aligned ( 1st byte is garbage due to PMP buffering), add space for AVI chunk header

        case s_camrestart:
            if(butstate) break; // in case trig held
            printf(cls butcol "EXIT  " whi inv " Camera " inv butcol "  Light" bot "Mode   ");
            
            if (explock) printf(inv "ExLock" inv);
            else printf("ExLock");

            if (vidmode) printf(tabx14 hspace "BMP" hspace inv "AVI" inv);
            else printf(tabx14 hspace inv "BMP" inv hspace "AVI");
            printf(taby11 tabx0 yel "%s", camnames[cammode]);
            camstate = s_camlive;
            cam_grabenable(camen_start, 7, 0);
            led1_off;
            break;

        case s_camlive:
            if (!cardmounted) camfile = 0;

            if (butpress & powerbut) {
                led1_off;
                cam_enable(0);
                camstate = s_camquit;
                break;
            }
            if (butpress & but1) {
                if (++cammode >= ncammodes) cammode = 1;
                cam_enable(cammode);
                camstate = s_camrestart;
                break;
            }
            if (butpress & but2) {
                explock ^= 1;
                cam_setreg(0x13, explock ? 0xe0 : 0xe7);
                camstate = s_camrestart;
                break;
            }
            if (butpress & but3) {
                camfile = 0;
                vidmode ^= 1;
                camstate = s_camrestart;
            }
            if (butpress & but4) if (led1) led1_on;
                else led1_off;
            if (butpress & but5) {
                camstate = vidmode ? s_camavistart : s_camgrab;
                break;
            }
            if (!cam_newframe) break;
            if (camflags & camopt_mono) monopalette(0, 255);
            cam_newframe = 0; // clear now in case display takes longer than cam frame time 
            dispimage(0, 12, xpixels, ypixels, (camflags & camopt_mono) ? (img_mono | img_revscan) : (img_rgb565 | img_revscan), cambuffer + 8);
            break;

        case s_avierr:
            if (!butpress) break;
            camstate = s_camrestart;
            break;

        case s_camavistart:
            printf(bot whi);
            camstate = s_camrestart; // in case error
            if (!cardmounted) {
                printf(inv"No Card         " inv del);
                break;
            }
            i = FSchdir("\\CAMVIDEO");
            if (i) {
                FSmkdir("CAMVIDEO");
                FSchdir("CAMVIDEO");
            }
            i = 0;
            do {
                docamname(camfile++, ct_avi);
                printf(bot "%-21s" tabx17 red inv "REC" inv whi, camname);
                fptr = FSfopen(camname, FS_READ);
                i = (fptr != NULL);
                if (i) FSfclose(fptr);
            } while (i);

            fptr = FSfopen(camname, FS_WRITE);
            FSchdir("\\");
            if (fptr == NULL) {
                printf(bot "Error FileOpen  " del del);
                FSfclose(fptr);
                break;
            }

            T5CON = 0b1000000001110000; // timer to measure grab+save time for playback framerate
            PR5 = 0xffff;
            TMR5 = 0; // timer on, /256 prescale
            IFS0bits.T5IF = 0; // detect roll

            rectime = 0;
            avi_bpp = camflags & camopt_mono ? 1 : 2;
            avi_width = xpixels;
            avi_height = ypixels;
            avi_framelen = xpixels * ypixels*avi_bpp;
            avi_frames = 0;
            avi_frametime = 200000; // dummy for now

            if (startavi()) {
                printf(bot "Error StartAVI  " del del);
                FSfclose(fptr);
                break;
            }

            cam_grabenable(camen_grab, 7, 0);
            campage=0;
            camstate = s_waitavi;
            break;


        case s_waitavi:

            if (avi_frames) if (butpress) {//ensure at least one frame to avoid creating dodgy file    
                cam_grabdisable();
                    printf(bot tabx12 "Ending");
                    avi_frametime = rectime / avi_frames; // get correct framerate on playback
                    if (finishavi()) {
                        printf(bot "Error EndAVI  " del del);
                        FSfclose(fptr);
                    }

                    camstate = s_camrestart;
                    break;
                }

            if (cam_newframe == 0) break; //got a new frame ?
        
            cam_grabenable(camen_grab,7+(campage?0:(avi_framelen+8)),0); // start grab to other page
            if (camflags & camopt_mono) monopalette(0, 255);

            // add chunk header before image data
            i=campage?(avi_framelen+8):0;
            cambuffer[i++] = '0';
            cambuffer[i++] = '0';
            cambuffer[i++] = 'd';
            cambuffer[i++] = 'c';
            cambuffer[i++] = avi_framelen;
            cambuffer[i++] = avi_framelen >> 8;
            cambuffer[i++] = 0;
            cambuffer[i++] = 0;
            
            dispimage(0, 12, xpixels, ypixels, (camflags & camopt_mono) ? (img_mono | img_revscan) : (img_rgb565 | img_revscan), cambuffer + i);

            if (avi_bpp == 1) flipcambuf(xpixels, ypixels, i); // mono AVIs have reverse scan direction
 
           
            if (FSfwrite(&cambuffer[i-8], avi_framelen + 8, 1, fptr) == 0) {
                printf(bot "Error:WriteFrame" del del);
                FSfclose(fptr);
                cam_grabdisable();
                break;
            }
            
            campage=campage?0:1; // page swap
            i = TMR5;
            if (IFS0bits.T5IF) i += 0x10000; // rolled - assume only once
            rectime += (i * 256 / (clockfreq / 1000000)); // uS
         
            printf(tabx0 taby11 yel "Frame %04d %4d secs", ++avi_frames, rectime / 1000000);
            TMR5 = 0;
            IFS0bits.T5IF = 0; 
      
            camstate = s_waitavi;

            break;

        case s_camgrab:
            printf(bot whi);
            camstate = s_camrestart; // default next state
            if (!cardmounted) {
                printf(inv"No Card         " inv del);
                break;
            }

            cam_grabdisable();

            i = FSchdir("\\CAMERA");
            if (i) {
                FSmkdir("CAMERA");
                FSchdir("CAMERA");
            }

            i = 0;
            do { // find first unused filename
                docamname(camfile++, ct_bmp);
                printf(bot "%-21s", camname);
                fptr = FSfopen(camname, FS_READ);
                i = (fptr != NULL);
                if (i) FSfclose(fptr);
            } while (i);

            if (!(camflags & camopt_mono)) conv16_24(xpixels * ypixels, 8); // RGB565 to 888

            fptr = FSfopen(camname, FS_WRITE);
            FSchdir("\\"); // exit dir for easier tidyup if error

            i = writebmpheader(xpixels, ypixels, (camflags & camopt_mono) ? 1 : 3);
            if (i == 0) {
                FSfclose(fptr);
                printf("Err writing header" bot "OK");
                camstate = s_camwait;
                break;
            }
            i = FSfwrite(&cambuffer[8], xpixels * ypixels * ((camflags & camopt_mono) ? 1 : 3), 1, fptr);
            FSfclose(fptr);
            if (i == 0) {
                printf("Err writing image" bot "OK");
                camstate = s_camwait;
                break;
            }


            break;

        case s_camwait:
            if (!butpress) break;
            camstate = s_camlive;
            cam_grabenable(camen_start, 7, 0);
            break;

        case s_camquit:
            camstate = s_camstart;
            cam_enable(0);
            return ("");

            break;


    } //switch camstate


    return (0);


}
