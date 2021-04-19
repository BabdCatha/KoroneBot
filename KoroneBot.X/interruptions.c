#include "interruptions.h"

/* Variable servant a effectuer les mesures
 sur l'ADC une fois sur dix seulement*/
unsigned char compteurADC = 0;

/*Variables servant a effectuer une moyenne
 sur plusieurs mesures pour la batterie*/
unsigned char nbMesures = 0;
unsigned char mesures[4];
unsigned char UBAT = 0;

#pragma code HighVector=0x08
void IntHighVector(void)
{
    _asm goto HighISR _endasm
}
#pragma code

#pragma interrupt HighISR
void HighISR(void)
{
    //Si Timer0 s'est declenche
    if(INTCONbits.TMR0IF){
        INTCONbits.TMR0IF = 0;

        //On surveille l'etat de la batterie
        survBatterie();

        //On recharge le Timer
        TMR0H = 0x67; //Valeurs à modifier
        TMR0L = 0x68;
    }
}

void survBatterie(void){
    if(compteurADC == 9){
        compteurADC = 0;
        /*On recupere la valeur precedente
         On utilise que 8 des 10 bits disponibles*/
        mesures[nbMesures] = ADRESH;
        nbMesures++;

        //Si on a assez de mesures
        if(nbMesures == 4){
            /*On fait la moyenne de cette maniere
             pour eviter un eventuel overflow*/
            UBAT = mesures[0]/4;
            UBAT += mesures[1]/4;
            UBAT += mesures[2]/4;
            UBAT += mesures[3]/4;

            //Si la tension est trop faible
            if(UBAT <= UMIN){
                //TODO
            }
        }

        //On relance une mesure de l'ADC
        ADCON0bits.GO = 1;
    }else{
        compteurADC++;
    }
}