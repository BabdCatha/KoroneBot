
/*******************************************************
*													   *
*				Gestionnaire I2C					   *
*													   *
*													   *
*													   *
*	Compilateur MCC18								   *
*	Auteur P. GENTRIC								   *
*	Version 1										   *
*******************************************************/

#include <p18f2520.h>	// Register definitions
#include <i2c.h>		// Librairie C18
#include "MI2C.h"

	// initialisation de la partie I2C
void MI2CInit(void)
{
	SSPSTAT = 0x80;				// Slew rate 100KHz	
	SSPCON1 = 0x28;				// Master Mode Enable, Sclock = FOSC/(4 * (SSPADD + 1)) I2C bus, 
	SSPCON2 = 0x00;
	SSPADD  = 0x13;				// Prediviseur pour SCL = 100KHz a 8MHz
}

/********************************************************************
*     Function Name:    Detecte_I2c                                 *
*     Return Value:     0 si detecte         							*
*     Parameters:       Adresse byte 				    			*
*     Description:      Detecte presence slave						*
* 																	* 
********************************************************************/
 char Detecte_i2c(char adresse_i2c)
	{	
  		while ((SSPCON2 & 0x1F) || (SSPSTATbits.R_W));  // ensure module is idle
		SSPCON2bits.SEN=1;        // initiate START condition
		while(SSPCON2bits.SEN);	 // Attente fin start condition
	
  		SSPBUF = adresse_i2c;		// Envoi adresse
		while(SSPSTATbits.BF);	// Attente buffer TX vide
		while(SSPCON2bits.ACKSTAT);	//Attente ACK slave
	 
		while ((SSPCON2 & 0x1F) || (SSPSTATbits.R_W));  // ensure module is idle                   
    	SSPCON2bits.PEN=1;                  // send STOP condition
    	while ( SSPCON2bits.PEN );  // attente fin stop condition
 
		PIR1bits.SSPIF = 0;				//Remise à jour du flag pour IT Stop  
  		return (0); 		// return with contents of SSPBUF	
	}


//---------------------------------------------------------------------
// SONAR_Read()
// Reads an integer value from SONAR connected to the adresse_i2c bus at
// the specified location MSB[0X02]. with ranging inch ou cm ou us
// return an integer distance [MSB,LSB]
//---------------------------------------------------------------------
int SONAR_Read( char adresse_i2c, char registre_distance)
{
union 
{
char b[2];
int i;
}
data;

union 
{
char b[2];
int i;
}
temp;

char error,retry;
error = 0;

retry = 10;								// Poll the SONAR up to 10 times MAX
do
	{
	error = EEAckPolling(adresse_i2c);
	retry--;
	} while(error && retry > 0); 

retry = 10;								// Attempt to read high byte of data
do											// up to 10 times MAX
	{
	temp.i = EERandomRead(adresse_i2c, registre_distance);
	retry--;
	} while(temp.b[1] && retry > 0);

if(temp.b[1]) data.b[0] = 0xFF;		// Make read result 0 if error
else data.b[1] = temp.b[0];		// Otherwise get the high byte of data

retry = 10;								// Attempt to read low byte of data
do											// up to 10 times MAX
	{
	temp.i = EERandomRead(adresse_i2c, registre_distance + 1);
	retry--;
	} while(temp.b[1] && retry > 0);

if(temp.b[1]) data.b[1] = 0xFF;		// Make read result 0 if error
else data.b[0] = temp.b[0];		// Otherwise get the low byte of data

return data.i;
}
//---------------------------------------------------------------------
// SONAR_Write()
// Writes a char commande value to SONAR connected to the adresse_i2c bus at
// the specified location 0x00 with commande ranging inch, us, or cm
//---------------------------------------------------------------------
void SONAR_Write( char adresse_i2c, char commande)
{
char error, retry;
error = 0;

retry = 10;								// Poll the SONAR up to 10 times MAX
do
	{
	error = EEAckPolling(adresse_i2c);
	retry--;
	} while(error && retry > 0); 

retry = 10;								// Attempt to write low byte of data
do											// up to 10 times MAX
	{
	error = EEByteWrite(adresse_i2c, 0x00, commande);
	retry--;
	} while(error && retry > 0);
	
}

/********************************************************************
*     Function Name:    Ecrire_i2c_Telecom                                	*
*     Return Value:     0 si OK         							*
*     Parameters:       Adresse byte, Pointeur sur octet a ecrire 	*
*     Description:      Ecrire un buffer formaté string				*
* 																	* 
********************************************************************/
	
	char Ecrire_i2c_Telecom(char adresse_i2c, char* Send_Buff)
	{
		char index = 0;
	  	while ((SSPCON2 & 0x1F) || (SSPSTATbits.R_W));  // ensure module is idle
		SSPCON2bits.SEN=1;        // initiate START condition
		while(SSPCON2bits.SEN);	 // Attente fin start condition
	
  		SSPBUF = adresse_i2c;		// Envoi adresse
		while(SSPSTATbits.BF);	// Attente buffer TX vide
		while(SSPCON2bits.ACKSTAT);	//Attente ACK slave
		
		do
		{
			while ((SSPCON2 & 0x1F) || (SSPSTATbits.R_W));  // ensure module is idle   		
	 		SSPBUF = Send_Buff[index];		// Ecrit octet
	 		while(SSPSTATbits.BF);	// Attente buffer TX vide
	 		while(SSPCON2bits.ACKSTAT);	//Attente ACK slave
	 	}while(Send_Buff[index++]);	
	 	
		while ((SSPCON2 & 0x1F) || (SSPSTATbits.R_W));  // ensure module is idle                   
    	SSPCON2bits.PEN=1;                  // send STOP condition
    	while ( SSPCON2bits.PEN );  // attente fin stop condition
 
		PIR1bits.SSPIF = 0;				//Remise à jour du flag pour IT Stop  
  		return (0); 		// return with contents of SSPBUF	 				
	}
	
/********************************************************************
*     Function Name:    Lire_i2c_Telecom                                	*
*     Return Value:     0 si OK         							*
*     Parameters:       Adresse byte, Pointeur sur octet lu 		*
*     Description:      Lire un buffer formaté string				*
* 																	* 
********************************************************************/
	
	char Lire_i2c_Telecom(char adresse_i2c, char* Recv_Buff)
	{
		char index = 0;
	  	while ((SSPCON2 & 0x1F) || (SSPSTATbits.R_W));  // ensure module is idle
		SSPCON2bits.SEN=1;        // initiate START condition
		while(SSPCON2bits.SEN);	 // Attente fin start condition
	
  		SSPBUF = adresse_i2c +1;		// Envoi adresse
		while(SSPSTATbits.BF);	// Attente buffer TX vide
		while(SSPCON2bits.ACKSTAT);	//Attente ACK slave
		
		
		do
		{
	 		while ((SSPCON2 & 0x1F) || (SSPSTATbits.R_W));  // ensure module is idle
	 	
	 		SSPCON2bits.RCEN=1;        // SSPBUF en reception
	 		while(!SSPSTATbits.BF);	// Attente buffer RX plein
	 		Recv_Buff[index] = SSPBUF;		// Ecrit octet
	 		if(SSPBUF)
	 			SSPCON2bits.ACKDT=0;        // initiate ACK
	 		else
	 			SSPCON2bits.ACKDT=1;        // initiate NoACK
	 		SSPCON2bits.ACKEN=1;
	 		while(SSPCON2bits.ACKEN);	// Attendre fin ACKEN
	 	}while(Recv_Buff[index++]);	//Tant que not \'0'	
	 	
		while ((SSPCON2 & 0x1F) || (SSPSTATbits.R_W));  // ensure module is idle                   
    	SSPCON2bits.PEN=1;                  // send STOP condition
    	while ( SSPCON2bits.PEN );  // attente fin stop condition
 
		PIR1bits.SSPIF = 0;				//Remise à jour du flag pour IT Stop  
  		return (0); 		// return with contents of SSPBUF	 				
	}


/********************************************************************
*     Function Name:    Write_PCF8574                               *
*     Return Value:     erreur 0 si OK         						*
*     Parameters:       Adresse_i2c byte, data byte 				*
*     Description:      Write data at current address of chip 8574  *
*                       device.										*
*                                                                   *  
********************************************************************/
char Write_PCF8574(char adresse_i2c, char data )
{
	char error = 0;	
  	while ((SSPCON2 & 0x1F) || (SSPSTATbits.R_W));  // ensure module is idle
	SSPCON2bits.SEN=1;        // initiate START condition
	while(SSPCON2bits.SEN);	 // Attente fin start condition
  	SSPBUF = adresse_i2c;		// Write adresse mode Ecriture un octet
	while(SSPSTATbits.BF);	// Attente buffer TX vide
	while(SSPCON2bits.ACKSTAT);	//Attente ACK slave

  	while ((SSPCON2 & 0x1F) || (SSPSTATbits.R_W));  // ensure module is idle
	SSPBUF = data;				//Ecrit data pour transmission
	while(SSPSTATbits.BF);			// Attente buffer TX vide
	while(SSPCON2bits.ACKSTAT);	//Attente ACK slave

  	while ((SSPCON2 & 0x1F) || (SSPSTATbits.R_W));  // ensure module is idle 
                    
    SSPCON2bits.PEN=1;                  // send STOP condition
    while ( SSPCON2bits.PEN );  // attente fin stop condition 
	PIR1bits.SSPIF = 0;				//Remise à jour du flag pour IT Stop  
  	return (error); 				// return with  error	
}

	