#include <stdio.h>
#include <stdlib.h>
#include <p18f2520.h>
#include "interruptions.h"
#include "init.h"

#pragma config OSC = INTIO67
#pragma config PBADEN = OFF, WDT = OFF, LVP = OFF, DEBUG = ON





void main(void) {

    initClock();
    initPWM();
    initTelecommande();

    return (0);
}