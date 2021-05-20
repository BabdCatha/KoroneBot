#include "interruptions.h"
#include <stdio.h>








etat etatGlobal={0, true, 0, 12, 0.0, 0, 0};


char texte[]="VBAT : XX.XX, phase = X\r\n\0";
unsigned char longueur=25;
unsigned char position=0;

/* Variable servant a effectuer les mesures
 sur l'ADC une fois sur dix seulement*/
unsigned char compteurADC = 0;

/*Variables servant a effectuer une moyenne
 sur plusieurs mesures pour la batterie*/
unsigned char nbMesures = 0;
unsigned char mesures[4];


//variable servant à définir la durée de la rotation, puis de l'avancée rectiligne en phase 2
unsigned char compteurPhase2;

char tamponLectureTelecommande[3]; //pour 0x31 puis 0x3y puis 00(='\0')
char compteurSerie=0; //jusque 20 (2s)

#pragma code HighVector=0x08
void IntHighVector(void)
{
    _asm goto HighISR _endasm
}
#pragma code



#pragma interrupt HighISR
void HighISR(void)
{
    if(INTCONbits.INT0IF==1)
    {
        //TELECOMMANDE
        Lire_i2c_Telecom(0xA2, tamponLectureTelecommande);
        if(tamponLectureTelecommande[1]==0x33 && etatGlobal.distanceSonar>=150) //signifie touche centre pressï¿½e et obstacle ï¿½ plus d'1m50
        {
            etatGlobal.phase=1;
            CCPR1L=VIT_CONTRAT_RECT+OFFSET_CCPR1L;
            CCPR2L=VIT_CONTRAT_RECT;
            PORTAbits.RA6=1; //sens de rotation
            PORTAbits.RA7=1;
            //vitesse PWM telle que 30cm.s-1 sur chaque moteur en marche avant
        }
        //FIN TELECOMMANDE
        INTCONbits.INT0IF=0; //doit ï¿½tre mis ï¿½ 0 manuellement
    }else if(INTCONbits.TMR0IF){
        //Si Timer0 s'est declenche
       
        //On surveille l'etat de la batterie
        survBatterie();
      
        //RS232
        if(compteurSerie>=20)
        {
            compteurSerie=0;
            position=0;
            texte[7] = etatGlobal.VBatPartEnt/10 + 0x30;
            texte[8] = (etatGlobal.VBatPartEnt%10) + 0x30;
            texte[10] = etatGlobal.VBatPartDec/10 + 0x30;
            texte[11] = (etatGlobal.VBatPartDec%10) + 0x30;
            texte[22] = etatGlobal.phase + 0x30;
            TXREG = texte[position];
            while(position<=longueur)
            {
                while(!PIR1bits.TXIF)
                {}
                PIR1bits.TXIF = 0;
                position++;
                TXREG =texte[position];
            }
            //printf("Valeur batterie: %d.%d V, Initialisation finie: %d, Phase:%d\r\n", etatGlobal.VBatPartEnt, etatGlobal.VBatPartDec, etatGlobal.initialisationEnCours, etatGlobal.phase);
        }
        else
        {
            compteurSerie++;
        }
        //FIN RS232

        //SONAR
        //mesure en cm car commande 0x51
        etatGlobal.distanceSonar=SONAR_Read(0xE0, 2); //0xE0 est l'adresse par défaut du Sonar et si l'on regarde la fonction SONAR_Read, elle lit d'abord l'octet fort (position 2) puis l'octet faible (position 3)
        SONAR_Write(0xE0, 0x51); //on demande une nouvelle mesure qui sera prête à lire à la prochaine interruption TIMER0, 100ms plus tard
        //NB: la fonction SONAR_Write s'occupe d'indiquer que l'on écrit au registre 0 du sonar, il suffit de lui spécifier la commande que l'on veut y écrire
        //FIN SONAR
        
        //GESTION DES PHASES
        if(etatGlobal.phase==1 && etatGlobal.distanceSonar<40)
        {
            etatGlobal.phase=2;//phase 2/rotation
            compteurPhase2=0;
            CCPR1L=VIT_ROTATION;
            CCPR2L=VIT_ROTATION;
            PORTAbits.RA6=1; //sens de rotation
            PORTAbits.RA7=0;
        }
        else if(etatGlobal.phase==2) //phase 2/rotation
        {
            if(compteurPhase2==dureeRotation)
            {
                etatGlobal.phase=3; //phase 2/rectiligne
                compteurPhase2=0;
                CCPR1L=VIT_CONTRAT_RECT+OFFSET_CCPR1L;
                CCPR2L=VIT_CONTRAT_RECT;
                PORTAbits.RA6=1; //sens de rotation
                PORTAbits.RA7=1;
            }
            else
            {
                compteurPhase2++;
            }
        }
        else if(etatGlobal.phase==3)
        {
            if(compteurPhase2==dureeRectiligne)
            {
                CCPR1L=0;
                CCPR2L=0; //on arrête les moteurs (fin du déplacement du robot, retour en idle)
                etatGlobal.phase=0;
            }
            else
            {
                compteurPhase2++;
            }
        }
        //FIN GESTION DES PHASES

        //On recharge le Timer
        TMR0H = 0x3C;
        TMR0L = 0xAF;
        INTCONbits.TMR0IF = 0; //doit être mis à 0 manuellement
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
            etatGlobal.VBatNum += mesures[0];
            etatGlobal.VBatNum += mesures[1];
            etatGlobal.VBatNum += mesures[2];
            etatGlobal.VBatNum += mesures[3];
            etatGlobal.VBatNum /=4;
            etatGlobal.VBatReel = ((float)etatGlobal.VBatNum)*3.2*5.0/255.0; //on multiplie VBatNum par un coefficient qui revient à défaire la quantification du CAN puis à défaire l'affaiblissement du pont diviseur de tension
            etatGlobal.VBatPartEnt=(int)etatGlobal.VBatReel; //on ne peut pas printf des float car la bibliothèque ne le permet pas (cela doublerait sa taille d'implémenter cette fonctionnalité, ainsi elle n'est pas disponible pour le PIC18F2520)
            etatGlobal.VBatPartDec=(etatGlobal.VBatReel-etatGlobal.VBatPartEnt)*100; //deux chiffres après la virgule
            //Si la tension est trop faible
            if(etatGlobal.VBatNum <= UMIN){
                etatGlobal.phase=-1;
                PORTBbits.RB5=1;
            }
            nbMesures=0;
        }

        //On relance une mesure de l'ADC
        ADCON0bits.GO = 1;
    }else{
        compteurADC++;
    }
}