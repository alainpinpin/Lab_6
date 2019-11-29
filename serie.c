 /**
 * @file   serie.c
 * @author Benoit Beaulieu
 * @date   Mai 2017
 * @brief  �criture des fonctions putch(), getch(), getche() et kbhit(). Les
 * prototypes des ces fonctions sont d�j� pr�vu dans conio.h du compilateur, 
 * mais on doit �crire le code.
 * Aussi, on trouve le code d'initialisation du port s�rie init_serie() dont
 * le prototype est d�fini dans serie.h.
 * @version 1.0
 * Environnement:
 *     D�veloppement: MPLAB X IDE (version 3.50)
 *     Compilateur: XC8 (version 1.40)
 *     Mat�riel: Carte d�mo du Pickit3. PIC 18F45K20
 */


#include <xc.h>
#include <conio.h>
#include "serie.h"

/**
 * @brief Initialise les registres pour utiliser le port s�rie EUSART
 *        TX = RC6, RX = RC7 (9600, n, 8 ,1)
 *        voir proc�dure page 238 des fiches techniques
 * @param aucun
 * @return aucun
 */
void init_serie(void)
{
    
    TRISCbits.TRISC7=1;
    TRISCbits.TRISC6=1;  

    //config de la vitesse (tableau 18-5 des fiches tech )
    //Fosc = 1MHz et on veut 9600 bauds. SYNC=0. BRGH=1. SPBRGH:SPBRG = 25
    SPBRGH=0x00;
    SPBRG=25;
    BAUDCONbits.BRG16=1;
    TXSTAbits.BRGH=1;
    
    
    TXSTAbits.SYNC=0;  //mode asynchrone
    TXSTAbits.TXEN=1;  //enable tx 
    RCSTAbits.CREN = 1; //enable rx
    RCSTAbits.SPEN=1;  //enable EUSART et config pin tx en sortie
    ANSEL = 0;         //mode analogique disable 
}    



/**
 * @brief  Envoi un caract�re sur le port s�rie 
 * @param  car: le caract�re � tx
 * @return Rien
 */
void putch(char car)
 {
    
    while(TXSTAbits.TRMT==0); //attend que buffer de tx est vide
    TXREG = car;       // Envoi le caract�re
    //delaiMs(10);        // d�lai de 10 ms
 }

/**
 * @brief  Lire un caract�re sur le port s�rie.
 * @param  Rien
 * @return Le caract�re lu sur le port s�rie
 */
char getch(void)
{
    unsigned char c;

    while (!RCIF)
        ;               // Attend que le port re�oive un caract�re
    c = RCREG;         // va chercher le caract�re du UART
    
    return c;
}

/**
 * @brief  Lire un caract�re sur le port s�rie et le renvoit (echo.
 * @param  Rien
 * @return Le caract�re lu sur le port s�rie
 */
char getche(void)
{
    unsigned char c;
    while (!RCIF);     // Attend que le port re�oive un caract�re
    c = RCREG;         // va chercher le caract�re du UART
    
    //echo
    while(TXSTAbits.TRMT==0); //attend que buffer de tx est vide
    TXREG = c;       // Echo

    return c;
}

/**
 * @brief  V�rifier si le port s�rie a re�u un caract�re.
 * @param  Rien
 * @return Le statut de l'�tat de r�ception, 1= caract�re pr�sent
 */
bit kbhit(void)
{
    return RCIF;
}






