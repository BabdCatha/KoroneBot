#include "interruptions.h"

typedef unsigned char bool;
#define true    1
#define false   0


#pragma code HighVector=0x08
void IntHighVector(void)
{
    _asm goto HighISR _endasm
}
#pragma code

char tamponLectureTelecommande[5]; //pour 0x31 puis 0x3y puis 00(='\0')
int distanceSonar=200;

typedef struct{
    bool phase1;
    bool phase2;
}etat;

etat etatGlobal={false, false};

#pragma interrupt HighISR
void HighISR(void)
{
    if(INTCONbits.INT0IF==1)
    {
        Lire_i2c_Telecom(0xA2, &tamponLectureTelecommande);
        if(tamponLectureTelecommande[3]==0x3 && distanceSonar>=150) //signifie touche centre pressée et obstacle à plus d'1m50
        {
            etatGlobal.phase1=true;
            CCPR1L=1; //à remplir epérimentalement
            CCPR2L=2;
            //vitesse PWM telle que 30cm.s-1 sur chaque moteur en marche avant
        }
        INTCONbits.INT0IF=0; //doit être mise à 0 manuellement
    }
}