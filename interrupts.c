
#include "cambadge.h"
#include "globals.h"
#include <sys/kmem.h> // for KVA_TO_PA macro for DMA logical-physical address translation


//_________________________________________________________  Everything to do with interrupts


 unsigned int lineskip,cambufptr;
 unsigned int xstartlat,xdivlat,ydivlat,xpixlat,ypixlat; // latched versions of camera parameters, sampled at vsync

//______________________________________________________________________________general exception trap

#if serialcontrol==1
 //-------------------------------------------------------------------------- UART RX
 
 void __ISR(_UART_2_VECTOR,IPL4SOFT) u2rxint(void)
 {// simple uart receive. Foreground task watches rxptr for completion and takes care of framing timeout & errors etc.
  IFS1CLR=_IFS1_U2RXIF_MASK;   

  while(U2STAbits.URXDA) {
    rxbuf[rxptr++]=U2RXREG;
    if(rxptr>=rxbufsize) rxptr=0; //buffer overflow
  }
  rxtimer=0; // reset framing timeout

 }
#endif
  //-------------------------------------------------------------------------- Vertical Sync INT4
 
 void __ISR(_EXTERNAL_4_VECTOR,IPL7SOFT) vsint(void)
{ // camera vertical sync - set everything up for grabbing a new frame  
    
 IFS0CLR=_IFS0_INT4IF_MASK;
 DCH0CON=0b0000000000000000; // DMA channel disable (in case overflow from previous frame) 
 while(DCH0CONbits.CHBUSY); // ensure any pending DMAs from overflowed previous frame finished. Assumes DMA1 done if DMA0 is

cam_busy=1; // status flags
if(!cam_wrap) cam_started=1; // force start to avoid FG task stalling if no transfer occurs due to bad parameters 
cam_wrap=0; 
cambufptr=camoffset;
linecnt=0;

//latch all parameters for next frame
lineskip=ystart;xstartlat=xstart; ydivlat=ydiv;xpixlat=xpixels;ypixlat=ypixels;
PR3=xdiv-1; // pixel clock prescale

// PMP works one byte behind - when PMDIN is read by DMA trigger, it reads the value latched by the last
// read, and latches a new value in from the external port ( this avoids wait states for the external data)
// This means the first byte is garbage, so image data starts at cambuffer+1
// numbering convention : even pixels are 0,2,4 etc.  odd pixels are 1,3,5 etc.
// "pixels" is one PIXCLK, which is actually half a pixel - BBBBBGGG or GGGRRR in RGB656, or the Y or U in YUV 

// in RGB mode, we transfer 2 bytes per DMA, and rely on the fact that two consecutive DMA cycles just happen to take two PIXCLK cycles
// this hugely simplifies dealing with the data, but locks the relationship between PIXCLK and the CPU clock
// If we wanted to read more slowly we could maybe add wait cycles to the PMP configuration



DMACONSET=1<<15; // setup DMA
DCH0SSA=KVA_TO_PA((void*)&PMDIN); // source address
DCH0SSIZ=1; 		// Source size
DCH0CSIZ=(camflags & camopt_double)?2:1;                 //=1 for mono 

// cell size
DCH0DSIZ=(camflags & camopt_mono)?xpixlat+2:xpixlat*2+2;         // transfer size for even pixels, +2 due to PMP working 1 byte behind 

           // transfer size for odd pixels
DCH0ECON=14<<8 | 1<<4;      // DMA trigger source for odd pixels : 14=Timer 3 , enable irq to start transfer


IEC0CLR=_IEC0_INT1IE_MASK; // clear hsync int 
IEC0SET=_IEC0_INT1IE_MASK; // enable hsync int
}

 
  //-------------------------------------------------------------------------- Horizontal Sync INT1

 void __ISR(_EXTERNAL_1_VECTOR,IPL6SOFT) hsint(void)
{ //camera horizontal sync int, occurs at end of line so pixel clock is not active
 unsigned int i;
   
IFS0CLR=_IFS0_INT1IF_MASK;  

if(lineskip--) return; // ignore lines for Y scaling
 lineskip=ydivlat-1;  

 if(linecnt==0) {cam_wrap=1;cam_started=1;} // set flags on first line to tell FG task capture has started 
 i=-xstartlat*2; 
 TMR3=i-1;  // timer offset to start of active part of line
  
 DCH0CON=0b0000000000000000; // DMA channel disable (in case overrun from last due to bad parameters) 
 while(DCH0CONbits.CHBUSY); // wait for any overrun to stop 
 DCH0DSA=KVA_TO_PA(&cambuffer[cambufptr]); // destination address 
 DCH0CON=0b0000000010000000; // DMA channel enable 

 DCH0INTCLR=0xff; // clear any pending DMA ints    
 IFS1CLR=_IFS1_DMA0IF_MASK;        
 IEC1SET=_IEC1_DMA0IE_MASK; // enable dma-complete int
}

//------------------------------------------------------------ dma complete int
 
 void __ISR(_DMA_0_VECTOR,IPL5SOFT) dmaint(void)
{ // DMA complete int - line of data received. 

IFS1CLR=_IFS1_DMA0IF_MASK;
IEC1CLR=_IEC1_DMA0IE_MASK;// disable DMA int
    
    camaddr=cambufptr; // for use by FG task if it wants to overlap cam grab and using data
    cambufptr+=(camflags & camopt_mono)?xpixlat:xpixlat*2;
    if(cambufptr>=cammax) cambufptr=camoffset; // wrap

 if(++linecnt==ypixlat) { // last line? 
     
    IEC0CLR=_IEC0_INT1IE_MASK; //     kill Hsync int  
    cam_busy=0;cam_newframe=1;   
    
    if(cam_stop) IEC0CLR = _IEC0_INT4IE_MASK; //
   
   } 

 }

 
//------------------------------------------------------------------------

void setupints(void)
{

//Vsync - start of camera frame
 IFS0CLR=_IFS0_INT4IF_MASK; 
 IPC4bits.INT4IP=7; 
 IEC0bits.INT4IE=0;   // gets enabled by cam_enable
 INTCONbits.INT4EP=1; // rising edge int

//Hsync - start of camera line
IPC1bits.INT1IP=6; 
IEC0bits.INT1IE=0; //  gets enabled by vsync
INTCONbits.INT1EP=0; // falling edge int

// line DMA complete, gets enabled by hsync int
DCH0INT=1<<21; // int on done
IPC10bits.DMA0IP=5;
#if serialcontrol==1
//Uart receive
IPC9bits.U2IP=4;
IFS1bits.U2RXIF=0;
IEC1bits.U2RXIE=1;
#endif
 INTCONSET=_INTCON_MVEC_MASK; 
 asm("nop"); // this seems to be necessary otherwise it crashes if there's a pending int when ints get enabled, 
             // which can happen with vsync ( silicon issue? Microchip haven't responded to my support case)
__builtin_enable_interrupts();   

    }  