// maps app numbers to function name. Functions themselves take care of displayed names 

// this list defines apps present and their order in menu
#define app_1 camera
#define app_2 browser
#define app_3 particle
#define app_4 tetrapuzz
#define app_5 breakout
#define app_6 puzzles
#define app_7 imagefx
#define app_8 codescan
#define app_9 scope
#define app_10 settings
#define app_11 demoapp

#define applist app_1,app_2, app_3,app_4,app_5,/*app_6,*/app_7,app_8,app_9,app_10,app_11

#define napps (sizeof(apps)/4)

// placeholder declarations - need one for each potential app

char* app_1(unsigned int);
char* app_2(unsigned int);
char* app_3(unsigned int);
char* app_4(unsigned int);
char* app_5(unsigned int);
char* app_6(unsigned int);
char* app_7(unsigned int);
char* app_8(unsigned int);
char* app_9(unsigned int);
char* app_10(unsigned int);
char* app_11(unsigned int);

