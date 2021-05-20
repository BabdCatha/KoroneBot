#include "init.h"

char tamponEcritureTelecommande[2]={0x31, 0x00};
void initTelecommandeSonar(void)
{
    TRISCbits.RC3=1; //SCL et SDA en entrÃ©e
    TRISCbits.RC4=1;
    MI2CInit(); //initialise certains paramï¿½tres I2C
    SONAR_Write(0xE0, 0x51);
    Ecrire_i2c_Telecom(0xA2, tamponEcritureTelecommande); //signifie ï¿½ U4(PIC16F1824) que l'on est en mode rï¿½ception et pas en mode test
}

void initRS232(void) //baud=9600, 8bits donnï¿½es, pas de bit de paritï¿½, un bit de stop, on ne fait que transmettre
{
    BAUDCONbits.BRG16=1; //baudrate en 16bits
    TXSTAbits.SYNC=0; //mode asynchrone
    TXSTAbits.BRGH=1; //mode grande vitesse

    //3 premiers paramï¿½tres => baudrate=Fosc/(4(n+1)) oï¿½ n=SPBRGH:SPBRG
    SPBRG=208; //baudrate=9600 b/s => n=207.33333 arrondi au supï¿½rieur => n=208
    SPBRGH=0;
    TRISCbits.RC6=1; //TX en entrï¿½e
    RCSTAbits.SPEN=1; //validation port sï¿½rie



}
void initInterruption(void)
{
   RCONbits.IPEN=0; //toutes les interruptions ont la mÃªme prioritÃ©
   INTCONbits.GIE=1; //autorise les interruptions

   TXSTAbits.TXEN=1; //transmission sï¿½rie autorisï¿½e

   INTCONbits.PEIE=1; //autorise les interruptions pÃ©riphÃ©rique
   INTCONbits.INT0IE=1; //autoriser l'interruption INT0 (commande tÃ©lÃ©commande prÃªte Ã  Ãªtre transmise)
   INTCONbits.TMR0IE = 1; //autoriser l'interrpution Timer0
}

void initClock(void)
{

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


    TRISAbits.RA6=0; //les bits sont à défaut à 0, mais c'est un rappel: il ne faut pas oublier que RA6 et RA7 sont utilisés pour indiquer le sens de rotation des moteurs
    TRISAbits.RA7=0;
    //On met les deux moteurs Ã  0
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

    ADCON1bits.VCFG0 = 0;//Vref+=VDD=5V
    ADCON1bits.VCFG1 = 0;//Vref-=VSS=0V
    ADCON1bits.PCFG = 12;//configuration des ANx en A ou D

    ADCON2bits.ACQT = 3;
    ADCON2bits.ADCS = 1; //Tad=1µs
    ADCON2bits.ADFM = 0;//on justifie à gauche, on ignore les deux bits de poids faible

    ADCON0bits.CHS = 2;
    ADCON0bits.ADON = 1;

}
