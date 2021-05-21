#ifndef INTERRUPTIONS_H
#define	INTERRUPTIONS_H

#include "MI2C.h"
#include <p18f2520.h>

//Tension minimale de la batterie
#define UMIN 0x9A

typedef unsigned char bool;
#define true    1
#define false   0


//Tension minimale de la batterie
#define UMIN 0x9A


#define VIT_CONTRAT_RECT 60 // correspond à 30cm.s-1
#define OFFSET_CCPR1L 6 //la chenille gauche tourne moins bien que la gauche, on vient corriger ça
#define VIT_ROTATION 40 //pour la phase 2

//valeur qui, multipliée par 0.1 (100ms) donne la durée en seconde de la rotation du robot en phase 2
#define dureeRotation 11

//valeur qui, multipliée par 0.1 (100ms) donne la durée en seconde de l'avancée rectiligne du robot en phase 3 (1 metre)
#define dureeRectiligne 31

typedef struct{
    char phase; //-1<=>batterie trop déchargée; 0<=>idel; 1<=>phase 1; 2<=>phase2 rotation; 3<=>phase2 avancï¿½e rectiligne
    bool initialisationEnCours;
    int distanceSonar; //en cm car mesure avec commande 0x51
    int VBatNum;
    float VBatReel;
    int VBatPartEnt;
    int VBatPartDec;
    char affichageLED;
}etat;

void HighISR(void);
void survBatterie(void);

#endif