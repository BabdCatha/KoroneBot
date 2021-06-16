#include "interruptions.h"
#include <stdio.h>
#include "init.h" //pour savoir si on est en DEBUG ou en RELEASE

//NB: tous les compteurs utilisent l'interruption de timer0 (base de temps de 100ms) pour gérer leurs évènements
//NB: pour la signification des registres CCPRXL et RAX lors de la gestion des phases, voir interruptions.h
etat etatGlobal={0, true, 0, 12, 0.0, 0, 0, 0};

#ifndef RELEASE
char texte[]="INIT FINI: XXX, VBAT : XX.XX, phase = X\r\n"; //chaîne mise à jour avec des informations de etatGlobal avant l'envoi par RS232
unsigned char longueur=40; //variables pour parcourir texte
unsigned char position=0; //de 0 à longueur inclus on envoie bien longueur + 1 caractères
#endif

/* Variable servant a effectuer les mesures
 sur l'ADC une fois sur dix seulement*/
unsigned char compteurADC = 0;

/*Variables servant a effectuer une moyenne
 sur plusieurs mesures pour la batterie*/
unsigned char nbMesures = 0;
unsigned char mesures[4];

char compteurAppuiBouton=-1;

//variable servant à définir la durée de la rotation, puis de l'avancée rectiligne en phase 2
unsigned char compteurPhase;

char tamponLectureTelecommande[3]; //pour 0x31 (1er octet transmis) puis 0x3y (second octet transmis, code télécommande) puis 00(='\0')
char compteurSerie=0; //variable servant à temporiser l'affichage série toute les 2s

#pragma code HighVector=0x08 //on ne s'occupe que du vecteur d'interruption 0x08 car on n'utilise pas les priorités d'interruptions (mode legacy)
void IntHighVector(void)
{
    _asm goto HighISR _endasm
}
#pragma code



#pragma interrupt HighISR
void HighISR(void)
{
    if(INTCONbits.INT0IF==1) //interruption télécommande
    {
        //TELECOMMANDE
        Lire_i2c_Telecom(0xA2, tamponLectureTelecommande);
        if(tamponLectureTelecommande[1]==0x33 && etatGlobal.distanceSonar>=150 && etatGlobal.phase==0) //signifie touche centre pressï¿½e et obstacle ï¿½ plus d'1m50
        {
            etatGlobal.phase=1;
            CCPR1L=VIT_CONTRAT_RECT+OFFSET_CCPR1L; //moteur gauche
            CCPR2L=VIT_CONTRAT_RECT; //moteur droit
            PORTAbits.RA6=0; //sens de rotation tel que le robot va vers l'avant
            PORTAbits.RA7=0;
            //vitesse PWM telle que 30cm.s-1 sur chaque moteur en marche avant
            etatGlobal.affichageLED = etatGlobal.affichageLED & 0b11111101;
            etatGlobal.affichageLED = etatGlobal.affichageLED | 0b00000100;
            compteurAppuiBouton=10;
        }
        else if(tamponLectureTelecommande[1]==0x33 && etatGlobal.phase>0 && compteurAppuiBouton==0)
        {
            CCPR1L=0;
            CCPR2L=0; //on arrête les moteurs (fin du déplacement du robot, retour en idle)
            etatGlobal.phase=0; //on peut remarquer qu'on ne rentre jamais dans cette boucle si phase=-1, donc il n'y a pas de problème à forcer phase à 0 ici
            PORTAbits.RA6=0; //sens de rotation
            PORTAbits.RA7=0;
            etatGlobal.affichageLED = etatGlobal.affichageLED & 0b00100001;
            etatGlobal.affichageLED = etatGlobal.affichageLED | 0b00000010;
        }
        //FIN TELECOMMANDE
        INTCONbits.INT0IF=0; //doit être mis à 0 manuellement
    }
    if(INTCONbits.TMR0IF){ //Si Timer0 s'est declenche

        if(compteurAppuiBouton>0)
        {
            compteurAppuiBouton--;
        }

        //On surveille l'etat de la batterie
        if(compteurADC == 9){
            compteurADC = 0;
            /*On recupere la valeur precedente
             On n'utilise que 8 des 10 bits disponibles*/
            mesures[nbMesures] = ADRESH;
            nbMesures++;

            //Si on a assez de mesures
            if(nbMesures == 4){
                etatGlobal.VBatNum=0;
                etatGlobal.VBatNum += mesures[0];
                etatGlobal.VBatNum += mesures[1];
                etatGlobal.VBatNum += mesures[2];
                etatGlobal.VBatNum += mesures[3];
                etatGlobal.VBatNum /= 4;
                etatGlobal.VBatReel = (((float)etatGlobal.VBatNum)*5.0/255.0)*3.3; //on multiplie VBatNum par un coefficient qui revient à défaire la quantification du CAN puis à défaire l'affaiblissement du pont diviseur de tension
                etatGlobal.VBatPartEnt=(int)etatGlobal.VBatReel; //on ne peut pas printf des float car la bibliothèque ne le permet pas (cela doublerait sa taille d'implémenter cette fonctionnalité, ainsi elle n'est pas disponible pour le PIC18F2520)
                etatGlobal.VBatPartDec=(etatGlobal.VBatReel-etatGlobal.VBatPartEnt)*100; //deux chiffres après la virgule
                if(etatGlobal.VBatNum <= UMIN){//Si la tension est trop faible
                    PORTBbits.RB5=1;
                    CCPR1L=0;
                    CCPR2L=0;
                    etatGlobal.phase=-1;
                    etatGlobal.affichageLED=0b00100001; //c'est le seul endroit où l'on "force" la valeur de affichageLED sans passer par des ET/OU logiques

                }
                nbMesures=0;
            }


        //DEBUT ADC
        //On relance une mesure de l'ADC
            ADCON0bits.GO = 1;
        }else{
            compteurADC++;
        }
        //FIN ADC
        //fin surveillance batterie

#ifndef RELEASE
        //RS232
        if(compteurSerie==20)
        {
            compteurSerie=0;
            position=0;
            if(etatGlobal.initialisationEnCours)
            {
                texte[11]='N';
                texte[12]='O';
                texte[13]='N';
            }
            else
            {
                texte[11]='O';
                texte[12]='U';
                texte[13]='I';
            }
            texte[23] = etatGlobal.VBatPartEnt/10 + 0x30; //dans cette partie on met à jour le string à envoyer par liaison série
            texte[24] = (etatGlobal.VBatPartEnt%10) + 0x30; //le "+ 0x30" est là pour obtenir le caractère ASCII associé au chiffre
            texte[26] = etatGlobal.VBatPartDec/10 + 0x30;
            texte[27] = (etatGlobal.VBatPartDec%10) + 0x30;
            if(etatGlobal.phase==-1)
            {
                texte[38] = 0x21; //il faudrait deux caractères pour afficher "-1", et plutôt que de modifier la chaîne toute entière, on prends pour convention que si le termina laffiche "phase !" cela signifie que la phase vaut -1, donc que la batterie n'est pas assez chargée
            }
            else
            {
                texte[38] = etatGlobal.phase + 0x30;
            }

            TXREG = texte[position];
            while(position<=longueur) //on envoie le string
            {
                while(!PIR1bits.TXIF)
                {}
                PIR1bits.TXIF = 0;
                position++;
                TXREG =texte[position];
            }
        }
        else
        {
            compteurSerie++;
        }
        //FIN RS232
#endif


        //AFFICHAGE LED
        //addresse du PCF8574 en binaire pour expliciter la correspondance avec la datasheet (A2/A1/A0 reliée à la masse)
        Write_PCF8574(0b01000000, ~etatGlobal.affichageLED); //il faut faire un NON logique sur affichageLED car sur le circuit, 0=LED allumée et 1=LED éteinte
        //FIN AFFICHAGE LED


        //SONAR
        if(etatGlobal.phase==0 || etatGlobal.phase==1) //on n'a pas besoin du sonar dans les autres phases
        {
            //mesure en cm car commande 0x51
            etatGlobal.distanceSonar=SONAR_Read(0xE0, 2); //0xE0 est l'adresse par défaut du Sonar et si l'on regarde la fonction SONAR_Read, elle lit d'abord l'octet fort (position 2) puis l'octet faible (position 3)
            SONAR_Write(0xE0, 0x51); //on demande une nouvelle mesure qui sera prête à lire à la prochaine interruption TIMER0, 100ms plus tard
            //NB: la fonction SONAR_Write s'occupe d'indiquer que l'on écrit au registre 0 du sonar, il suffit de lui spécifier la commande que l'on veut y écrire

        }
        //FIN SONAR
        
        //GESTION DES PHASES
        if(etatGlobal.phase==1 && etatGlobal.distanceSonar<40)
        {
            etatGlobal.phase=2;//phase 2/rotation
            etatGlobal.affichageLED = etatGlobal.affichageLED & 0b01111011;
            etatGlobal.affichageLED = etatGlobal.affichageLED | 0b01001000;
            compteurPhase=0;
            CCPR1L=VIT_ROTATION;
            CCPR2L=VIT_ROTATION;
            PORTAbits.RA6=0; //sens de rotation
            PORTAbits.RA7=1;
        }
        else if(etatGlobal.phase==2) //phase 2/rotation
        {
            if(compteurPhase==dureeRotation)
            {
                etatGlobal.phase=3; //phase 3/rectiligne
                etatGlobal.affichageLED = etatGlobal.affichageLED & 0b00110111;
                etatGlobal.affichageLED = etatGlobal.affichageLED | 0b00010000;
                compteurPhase=0;
                CCPR1L=VIT_CONTRAT_RECT+OFFSET_CCPR1L;
                CCPR2L=VIT_CONTRAT_RECT;
                PORTAbits.RA6=0; //sens de rotation
                PORTAbits.RA7=0;
            }
            else
            {
                compteurPhase++;
            }
        }
        else if(etatGlobal.phase==3)
        {
            if(compteurPhase==dureeRectiligne)
            {
                CCPR1L=0;
                CCPR2L=0; //on arrête les moteurs (fin du déplacement du robot, retour en idle)
                etatGlobal.phase=0;
                PORTAbits.RA6=0; //sens de rotation
                PORTAbits.RA7=0;
                etatGlobal.affichageLED = etatGlobal.affichageLED & 0b00101111;
                etatGlobal.affichageLED = etatGlobal.affichageLED | 0b00000010;
            }
            else
            {
                compteurPhase++;
            }
        }
        //FIN GESTION DES PHASES

        //On recharge le Timer avec Recharge = 15535 = 0x3CAF
        TMR0H = 0x3C;
        TMR0L = 0xAF;
        INTCONbits.TMR0IF = 0; //doit être mis à 0 manuellement
    }
}
