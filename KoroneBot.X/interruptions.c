#include "interruptions.h"
#include <stdio.h>

typedef unsigned char bool;
#define true    1
#define false   0



//Tension minimale de la batterie
#define UMIN 0x9A
typedef struct{
    char phase; //-1<=>batterie trop déchargée; 1<=>phase 1; 2<=>phase2 rotation; 3<=>phase2 avancï¿½e rectiligne
    bool initialisationEnCours;
    int distanceSonar;
    unsigned char VBatNum;
    float VBatReel;
}etat;

etat etatGlobal={0, true, 0, 12, 0.0};


/* Variable servant a effectuer les mesures
 sur l'ADC une fois sur dix seulement*/
unsigned char compteurADC = 0;

/*Variables servant a effectuer une moyenne
 sur plusieurs mesures pour la batterie*/
unsigned char nbMesures = 0;
unsigned char mesures[4];


#pragma code HighVector=0x08
void IntHighVector(void)
{
    _asm goto HighISR _endasm
}
#pragma code

char tamponLectureTelecommande[3]; //pour 0x31 puis 0x3y puis 00(='\0')
char compteurSerie=0; //jusque 20 (2s)


#pragma interrupt HighISR
void HighISR(void)
{
    if(INTCONbits.INT0IF==1)
    {
        //I2C
        Lire_i2c_Telecom(0xA2, tamponLectureTelecommande);
        if(tamponLectureTelecommande[1]==0x33 && etatGlobal.distanceSonar>=150) //signifie touche centre pressï¿½e et obstacle ï¿½ plus d'1m50
        {
            etatGlobal.phase=1;
            CCPR1L=1; //ï¿½ remplir epï¿½rimentalement
            CCPR2L=2;
            //vitesse PWM telle que 30cm.s-1 sur chaque moteur en marche avant
        }
        //FIN I2C
        INTCONbits.INT0IF=0; //doit ï¿½tre mis ï¿½ 0 manuellement
    }else if(INTCONbits.TMR0IF){
        //Si Timer0 s'est declenche
        INTCONbits.TMR0IF = 0;

        //On surveille l'etat de la batterie
        survBatterie();
      
        //RS232
        if(compteurSerie==20)
        {
            compteurSerie=0;
            printf("Valeur batterie: %f V, Initialisation finie: %d, Phase:%d\r\n", etatGlobal.VBatReel, etatGlobal.initialisationEnCours, etatGlobal.phase);
        }
        else
        {
            compteurSerie++;
        }
        //FIN RS232

        //On recharge le Timer
        TMR0H = 0x3C;
        TMR0L = 0xAF;
    }
}

void survBatterie(void){
    if(compteurADC == 9){
        compteurADC = 0;
        /*On recupere la valeur precedente
         On n'utilise que 8 des 10 bits disponibles*/
        mesures[nbMesures] = ADRESH;
        nbMesures++;

        //Si on a assez de mesures
        if(nbMesures == 4){
            /*On fait la moyenne de cette maniere
             pour eviter un eventuel overflow*/
            etatGlobal.VBatNum=0;
            etatGlobal.VBatNum += mesures[0]/4;
            etatGlobal.VBatNum += mesures[1]/4;
            etatGlobal.VBatNum += mesures[2]/4;
            etatGlobal.VBatNum += mesures[3]/4;
            etatGlobal.VBatReel = (etatGlobal.VBatNum); //on multiplie VBatNum par un coefficient qui revient à défaire la quantification du CAN puis à défaire l'affaiblissement du pont diviseur de tension

            //Si la tension est trop faible
            if(etatGlobal.VBatNum <= UMIN){
                etatGlobal.phase=-1;
            }
        }

        //On relance une mesure de l'ADC
        ADCON0bits.GO = 1;
    }else{
        compteurADC++;
    }
}