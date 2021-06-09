#ifndef INIT_H
#define	INIT_H
#include "MI2C.h"
#include <p18f2520.h>

//cette ligne doit être commentée en mode DEBUG
//#define RELEASE
//cette ligne doit être commentée en mode DEBUG



//init.c contient des commentaires détaillés
void initClock(void);
void initPWM(void);
void initTimer0(void);
void initADC(void);
void initInterruption(void);
#ifndef RELEASE
void initRS232(void);
#endif
void initTelecommandeSonar(void);

#endif