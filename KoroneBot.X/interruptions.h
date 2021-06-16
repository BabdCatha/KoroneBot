#ifndef INTERRUPTIONS_H
#define	INTERRUPTIONS_H

#include "MI2C.h"
#include <p18f2520.h>

//on définit notre propre type booléen pour produire du code plus lisible (stdbool.h n'est pas disponible)
typedef unsigned char bool;
#define true    1
#define false   0


//Tension minimale de la batterie
#define UMIN 0x66 //correspond à la valeur lue au CAN, lorsque la tension du pont diviseur de tension est telle que VBAT=7.2V (limite choisie pour le bon fonctionnement du robot
//attention, on ne lit que les 8 MSB du CAN

//pour indication, valeur max pour CCPRxL: 62, qui correspond à un rapport cyclique de 50% (car CCPRXL:DCXB1:DCXB0 = 250 au maximum (50%), ce qui divisé par 4 donne 62)
//de plus, CCPR1L et RA7 contrôlent la chenille gauche, et CCPR2L et RA6 contrôlent la chenille droite
//RA6/7 à 0 fait tourner la chenille correspondante telle que le robot va vers l'avant, et à 1 fait tourner la chenille correspondante telle que le robot va vers l'arrière
#define VIT_CONTRAT_RECT  50// correspond à 30cm.s-1 pour la phase de ligne droite
#define OFFSET_CCPR1L 10 //la chenille gauche tourne moins bien que la droite, on vient corriger ça
#define VIT_ROTATION 40 //pour la phase 2

//valeur qui, multipliée par 0.1 (100ms) donne la durée en seconde de la rotation du robot en phase 2
#define dureeRotation 10

//valeur qui, multipliée par 0.1 (100ms) donne la durée en seconde de l'avancée rectiligne du robot en phase 3 (1 metre)
#define dureeRectiligne 31

typedef struct{
    char phase; /*-1 <=> batterie insuffisante, bloque le robot jusqu'à son redémarrage avec une batterie rechargée;
                 * 0<=>idle (en attente de pression télécommande);
                 * 1<=>phase 1 (avancée rectiligne jusqu'à moins de 40cm de la cible;
                 * 2<=>phase 2 rotation;
                 * 3<=>phase 2 avancée rectiligne sur 1m; après phase 2, retour à phase 0
                 */
    bool initialisationEnCours; //état de cette variable communiqué par RS232 et par l'allumage de la LED D1 quand l'initialisation est terminée
    int distanceSonar; //en cm car mesure avec commande 0x51
    int VBatNum; //valeur numérique de la tension de batterie lue au CAN
    float VBatReel; //valeur en Volts de la tension de batterie
    int VBatPartEnt; //parties entières et décimales de VBatReel, utilisées pour l'affichage par RS232
    int VBatPartDec;
    char affichageLED; //affichageLED RA6:RA7:PHASE-1:PHASE2LIN:PHASE2ROT:PHASE1:PHASE0IDLE:INITOK
                       //             D8 :D7 :D6     :D5       :D4       :D3    :D2        :D1
                       //affichageLED est envoyé au PCF8574
}etat; //en mode RELEASE, la structure d'état global est toujours mise à jour, mais certaines informations ne sont pas utilisées car elles sont destinées à être envoyées par RS232

void HighISR(void);

#endif