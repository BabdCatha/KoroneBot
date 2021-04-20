#include "interruptions.h"
#include <stdio.h>

typedef unsigned char bool;
#define true    1
#define false   0


#pragma code HighVector=0x08
void IntHighVector(void)
{
    _asm goto HighISR _endasm
}
#pragma code

char tamponLectureTelecommande[3]; //pour 0x31 puis 0x3y puis 00(='\0')
char compteurSerie=0; //jusque 20 (2s)

typedef struct{
    char phase; //1<=>phase 1; 2<=>phase2 rotation; 3<=>phase2 avancée rectiligne
    bool initialisationEnCours;
    int distanceSonar;
    char VBat;
}etat;

etat etatGlobal={1, false, 200, 10};

#pragma interrupt HighISR
void HighISR(void)
{
    if(INTCONbits.INT0IF==1)
    {
        //I2C
        Lire_i2c_Telecom(0xA2, tamponLectureTelecommande);
        if(tamponLectureTelecommande[1]==0x33 && etatGlobal.distanceSonar>=150) //signifie touche centre pressée et obstacle à plus d'1m50
        {
            etatGlobal.phase=1;
            CCPR1L=1; //à remplir epérimentalement
            CCPR2L=2;
            //vitesse PWM telle que 30cm.s-1 sur chaque moteur en marche avant
        }
        //FIN I2C
        INTCONbits.INT0IF=0; //doit être mis à 0 manuellement
    }
    if(INTCONbits.TMR0IF==1)
    {
        //RS232
        if(compteurSerie==20)
        {
            compteurSerie=0;
            printf("Valeur batterie: %d V, Initialisation finie: %d, Phase:%d\r\n", etatGlobal.VBat, etatGlobal.initialisationEnCours, etatGlobal.phase);
        }
        else
        {
            compteurSerie++;
        }
        //FIN RS232



        TMR0IF=0;//doit être mis à 0 manuellement
    }
}