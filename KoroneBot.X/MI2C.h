#ifndef _MI2C_H_
#define _MI2C_H_


void MI2CInit(void);
char Detecte_i2c(char adresse_i2c);

// Pour les transactions du SONAR																		
void SONAR_Write( char adresse_i2c, char commande);
int SONAR_Read( char adresse_i2c, char registre_ditance);

// Pour la télécommande
char Lire_i2c_Telecom(char adresse_i2c, char* Recv_Buff);
char Ecrire_i2c_Telecom(char adresse_i2c, char* Send_Buff);

// Pour les transactions des PCF8574
char Write_PCF8574(char adresse_i2c, char data );

#endif