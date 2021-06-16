#include "init.h"
#include "interruptions.h"
#include <stdio.h>
char tamponEcritureTelecommande[2]={0x31, 0x00}; //utilisé pour envoyer le message à U4 (qui gère la télécommande) lors de son initialisation


extern etat etatGlobal; //structure mise à jour au fur et à mesure du fonctionnement du robot

void initTelecommandeSonar(void)
{
    TRISCbits.RC3=1; //SCL et SDA en entrée
    TRISCbits.RC4=1;
    MI2CInit(); //initialise certains paramètres I2C
    SONAR_Write(0xE0, 0x51); //on demande une première mesure au Sonar
    Ecrire_i2c_Telecom(0xA2, tamponEcritureTelecommande); //signifie à U4(PIC16F1824) que l'on est en mode réception et pas en mode test
}
#ifndef RELEASE
void initRS232(void) //baud=9600, 8bits données, pas de bit de parité, un bit de stop, on ne fait que transmettre
{
    BAUDCONbits.BRG16=1; //baudrate en 16bits
    TXSTAbits.SYNC=0; //mode asynchrone
    TXSTAbits.BRGH=1; //mode grande vitesse

    //3 premiers paramètres => baudrate=Fosc/(4(n+1)) où n=SPBRGH:SPBRG
    SPBRG=208; //baudrate=9600 b/s => n=207.33333 arrondi au supï¿½rieur => n=208
    SPBRGH=0;
    TRISCbits.RC6=1; //TX en entrée
    RCSTAbits.SPEN=1; //validation port série
    TXSTAbits.TXEN=1; //transmission série autorisée
}
#endif

void initInterruption(void)
{
   RCONbits.IPEN=0; //toutes les interruptions ont la même priorité
   INTCONbits.GIE=1; //autorise les interruptions
   INTCONbits.PEIE=1; //autorise les interruptions périphériques
   INTCONbits.INT0IE=1; //autoriser l'interruption INT0 (commande télécommande prête à être lue)
   INTCONbits.TMR0IE = 1; //autoriser l'interruption Timer0
}

void initClock(void)
{

    //On regle la frequence d'horloge sur 8MHz
    OSCCONbits.IRCF0 = 1;
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF2 = 1;

    OSCCONbits.IDLEN = 1; //passe en mode idle sur instruction SLEEP

}

void initPWM(void){

    //On met la valeur du prescaler a 16
    T2CONbits.T2CKPS1 = 1;
    T2CONbits.T2CKPS0 = 0;

    //Valeur calculée auparavant
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


    TRISAbits.RA6=0; //les bits sont à défaut à 0, mais c'est un rappel: il ne faut pas oublier que RA6 et RA7 sont utilisés pour indiquer le sens de rotation des moteurs
    TRISAbits.RA7=0;
    //On met les deux moteurs à vitesse nulle
    CCPR1L = 0;
    CCPR2L = 0;

}

void initTimer0(void){

    T0CONbits.T08BIT = 0; //timer 0 en mode 16 bits
    T0CONbits.T0CS = 0; //utilisation de l'horloge interne
    T0CONbits.PSA = 0; //on utilise le prescaler
    T0CONbits.T0PS = 1; //prescaler de 4 (recharge de 15535 (0x3CAF)) pour une base de temps de 100ms

    //On active le Timer0
    T0CONbits.TMR0ON = 1;

}

void initADC(void){

    ADCON1bits.VCFG0 = 0;//Vref+=VDD=5V
    ADCON1bits.VCFG1 = 0;//Vref-=VSS=0V
    ADCON1bits.PCFG = 12;//configuration des ANx en A ou D

    ADCON2bits.ACQT = 3; //Tacq=6*Tad=6µs
    ADCON2bits.ADCS = 1; //Tad=1µs
    ADCON2bits.ADFM = 0;//on justifie à gauche, on ignore les deux bits de poids faible

    ADCON0bits.CHS = 2;  //sélection de la patte AN2 comme entrée pour le CAN
    ADCON0bits.ADON = 1; //ADC en fonctionnement
    ADCON0bits.GO = 1;   //On lance une premiere mesure

}
