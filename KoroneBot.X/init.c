#include "init.h"

void initClock(void){

    //On regle la frequence d'horloge sur 8MHz
    OSCCONbits.IRCF0 = 0;
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF2 = 1;

}

void initPWM(void){

    //On met la valeur du prescaler a 16
    T2CONbits.T2CKPS1 = 1;
    T2CONbits.T2CKPS0 = 0;

    PR2 = 124;

    //On met les broches correspondantes en mode sortie
    TRISCbits.RC2 = 0;
    TRISCbits.RC1 = 0;

    //On active le Timer2
    T2CONbits.TMR2ON = 1;

    //On active le mode PWM
    CCP2CONbits.CCP2M3 = 1;
    CCP2CONbits.CCP2M2 = 1;

    /*On ne tiendra pas compte des deux derniers bits
    pour le mode PWM, on perd un peu en precision,
    mais cela rend les calculs plus simples*/
    CCP2CONbits.DC2B1 = 0;
    CCP2CONbits.DC2B0 = 0;
    CCP1CONbits.DC1B1 = 0;
    CCP1CONbits.DC1B0 = 0;

    //On met les deux moteurs à 0
    CCPR1L = 0;
    CCPR2L = 0;

}