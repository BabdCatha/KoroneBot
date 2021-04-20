#ifndef INTERRUPTIONS_H
#define	INTERRUPTIONS_H

#include <p18f2520.h>

//Tension minimale de la batterie
#define UMIN 0x9A

void HighISR(void);
void survBatterie(void);

#endif