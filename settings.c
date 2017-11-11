
#include "cambadge.h"
#include "globals.h"
char* settings(unsigned int action)
{
if(action==act_name) return("SETTINGS"); else if (action==act_help) return("Set user options"); 

if(action==act_start) printf(top butcol "EXIT  " whi inv "Settings" inv butcol "   Boot\n\n\nComing soon!");
 if(action!=act_poll) return(0);

if(!butpress) return(0);
if(butpress & powerbut) return("");
if(butpress & but4) {
      __builtin_disable_interrupts();  
    SYSKEY = 0x00000000; //write invalid key to force lock
     SYSKEY = 0xAA996655; //write key1 to SYSKEY
     SYSKEY = 0x556699AA; //write key2 to SYSKEY
     /* set SWRST bit to arm reset */
     RSWRSTSET = 1;
     /* read RSWRST register to trigger reset */
     unsigned int dummy;
     dummy = RSWRST;
     /* prevent any unwanted code execution until reset occurs*/
     while(1);   
    
}
return(0);
}