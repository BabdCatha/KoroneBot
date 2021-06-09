#include <stdio.h>
#include <stdlib.h>
#include <p18f2520.h>
#include "interruptions.h"
#include "init.h"

#pragma config OSC = INTIO67
#pragma config PBADEN = OFF, WDT = OFF, LVP = OFF, DEBUG = ON


extern etat etatGlobal; //structure mise à jour au fur et à mesure du fonctionnement du robot
void main(void) {

    initClock(); //Fréquence de 8MHz
    initPWM(); //F=1kHz et rapport cyclique < 50%
    initADC(); //Tad=1µs
    initTelecommandeSonar(); //entre autres, dire à U4(PIC16F1824) qu'on est en mode réception et pas en mode test
    initRS232(); //9600 baud, 8 bits, 1 stop, pas de bit de parité
    initTimer0(); //base temporelle de 100ms
    initInterruption(); //Interruption INT0 (télécommande), TMR0 (overflow)
    
    etatGlobal.initialisationEnCours=false; //initialisation terminée
    etatGlobal.phase=0; //phase idle (en attente de commande), voir struct etat
    etatGlobal.affichageLED=0b00000011; //voir explications à la définition de struct etat
    Nop(); //signifie "n'exécute aucune instruction", et nécessaire sinon les trois lignes précédentes ne sont pas lues (voir rapport)

    while(1){
        //Sleep(); //passe en mode IDLE quand il n'y a rien à faire, jusqu'à la prochaine interruption
        //ATTENTION: ce mode IDLE n'est pas modélisé par proteus, qui le agit TOUJOURS comme si on était en mode SLEEP complet
    }
}

