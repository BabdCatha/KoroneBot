#include "init.h"

char tamponEcritureTelecommande[2]={0x31, 0x00};
void initTelecommande()
{
    TRISCbits.RC3=1; //SCL et SDA en entrée
    TRISCbits.RC4=1;
    MI2CInit(); //initialise certains param�tres I2C
    Ecrire_i2c_Telecom(0xA2, tamponEcritureTelecommande); //signifie � U4(PIC16F1824) que l'on est en mode r�ception et pas en mode test
}

void initRS232() //baud=9600, 8bits donn�es, pas de bit de parit�, un bit de stop, on ne fait que transmettre
{
    BAUDCONbits.BRG16=1; //baudrate en 16bits
    TXSTAbits.SYNC=0; //mode asynchrone
    TXSTAbits.BRGH=1; //mode grande vitesse

    //3 premiers param�tres => baudrate=Fosc/(4(n+1)) o� n=SPBRGH:SPBRG
    SPBRG=208; //baudrate=9600 b/s => n=207.33333 arrondi au sup�rieur => n=208
    SPBRGH=0;
    TRISCbits.RC6=1; //TX en entr�e
    RCSTAbits.SPEN=1; //validation port s�rie



}
void initInterruption()
{
   RCONbits.IPEN=0; //toutes les interruptions ont la même priorité
   INTCONbits.GIE=1; //autorise les interruptions

   TXSTAbits.TXEN=1; //transmission s�rie autoris�e

   INTCONbits.PEIE=1; //autorise les interruptions périphérique
   INTCONbits.INT0IE=1; //autoriser l'interruption INT0 (commande télécommande prête à être transmise)
   INTCONbits.TMR0IE = 1; //autoriser l'interrpution Timer0
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

void initTimer0(void){

    T0CONbits.T08BIT = 0;
    T0CONbits.T0CS = 0;
    T0CONbits.PSA = 0;
    T0CONbits.T0PS = 1;

    //On active le Timer0
    T0CONbits.TMR0ON = 1;

}

void initADC(void){

    ADCON1bits.VCFG0 = 0;
    ADCON1bits.VCFG1 = 0;
    ADCON1bits.PCFG = 12;

    ADCON2bits.ACQT = 3;
    ADCON2bits.ADCS = 4;
    ADCON2bits.ADFM = 0;

    ADCON0bits.CHS = 2;
    ADCON0bits.ADON = 1;

}
