#ifndef INIT_H
#define	INIT_H
#include "MI2C.h"
#include <p18f2520.h>

//init.c contient les commentaires détaillés
void initClock(void);
void initPWM(void);
void initTimer0(void);
void initADC(void);
void initInterruption(void);
void initRS232(void);
void initTelecommandeSonar(void);

#endif