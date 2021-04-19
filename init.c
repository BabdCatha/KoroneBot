#include "init.h"

char tamponEcritureTelecommande[2]={0x31, 0x00};
void initTelecommande()
{
    TRISCbits.RC3=1; //SCL et SDA en entrée
    TRISCbits.RC4=1;
    MI2CInit();
    Ecrire_i2c_Telecom(0xA2, tamponEcritureTelecommande);
}
void initInterruption()
{
   RCONbits.IPEN=0; //toutes les interruptions ont la même priorité
   INTCONbits.GIE=1; //autorise les interruptions
   INTCONbits.PEIE=1; //autorise les interruptions périphérique
   INTCONbits.INT0IE=1; //autoriser l'interruption INT0 (commande télécommande prête à être transmise)
}

void initClock(void){

    //On regle la frequence d'horloge sur 8MHz
    OSCCONbits.IRCF0 = 1;
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF2 = 1;

}

void initPWM(void){

    //On met la valeur du prescaler a 16
    T2CONbits.T2CKPS1 = 1;
    T2CONbits.T2CKPS0 = 0;

    //Valeur calculee auparavant
    PR2 = 124;

    //On met les broches correspondantes en mode sortie
    TRISCbits.RC2 = 0;
    TRISCbits.RC1 = 0;

    //On active le Timer2
    T2CONbits.TMR2ON = 1;

    //On active le mode PWM
    CCP2CONbits.CCP2M3 = 1;
    CCP2CONbits.CCP2M2 = 1;
    CCP1CONbits.CCP1M3 = 1;
    CCP1CONbits.CCP1M2 = 1;

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