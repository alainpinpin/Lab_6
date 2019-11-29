 /**
 * @file   serie.c
 * @author Benoit Beaulieu
 * @date   Mai 2017
 * @brief  Écriture des fonctions putch(), getch(), getche() et kbhit(). Les
 * prototypes des ces fonctions sont déjà prévu dans conio.h du compilateur, 
 * mais on doit écrire le code.
 * Aussi, on trouve le code d'initialisation du port série init_serie() dont
 * le prototype est défini dans serie.h.
 * @version 1.0
 * Environnement:
 *     Développement: MPLAB X IDE (version 3.50)
 *     Compilateur: XC8 (version 1.40)
 *     Matériel: Carte démo du Pickit3. PIC 18F45K20
 */


#include <xc.h>
#include <conio.h>
#include "serie.h"

/**
 * @brief Initialise les registres pour utiliser le port série EUSART
 *        TX = RC6, RX = RC7 (9600, n, 8 ,1)
 *        voir procédure page 238 des fiches techniques
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
 * @brief  Envoi un caractère sur le port série 
 * @param  car: le caractère à tx
 * @return Rien
 */
void putch(char car)
 {
    
    while(TXSTAbits.TRMT==0); //attend que buffer de tx est vide
    TXREG = car;       // Envoi le caractère
    //delaiMs(10);        // délai de 10 ms
 }

/**
 * @brief  Lire un caractère sur le port série.
 * @param  Rien
 * @return Le caractère lu sur le port série
 */
char getch(void)
{
    unsigned char c;

    while (!RCIF)
        ;               // Attend que le port reçoive un caractère
    c = RCREG;         // va chercher le caractère du UART
    
    return c;
}

/**
 * @brief  Lire un caractère sur le port série et le renvoit (echo.
 * @param  Rien
 * @return Le caractère lu sur le port série
 */
char getche(void)
{
    unsigned char c;
    while (!RCIF);     // Attend que le port reçoive un caractère
    c = RCREG;         // va chercher le caractère du UART
    
    //echo
    while(TXSTAbits.TRMT==0); //attend que buffer de tx est vide
    TXREG = c;       // Echo

    return c;
}

/**
 * @brief  Vérifier si le port série a reçu un caractère.
 * @param  Rien
 * @return Le statut de l'état de réception, 1= caractère présent
 */
bit kbhit(void)
{
    return RCIF;
}






