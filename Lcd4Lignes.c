/**
 *  Fichier:  Lcd4Lignes.c
 *  Date:     D�cembre 2012
 *  Auteur:   Christian Francoeur
 *  Revision: Benoit Beaulieu (5 sept 2018)
 *
 *  Description: R��criture pour MPLAB X et CX8 C Compiler
 *
 *               Les caract�res suivants sont g�n�r�s et inscrit dans les cases de 0 -> 7
 *                  � => 0         � => 1         � => 2        � => 3        � => 4
 *                  � => 5        � => 6        � => 7
 *
 *               On peut utiliser LcdEcritCar() pour afficher ces caract�res:
 *                  Ex: LcdEcritCar(0xE0)    affichera un a
 *
 *               On peut aussi utiliser LcdPutMess() et inclure ces caract`res dans la cha�ne:
 *                  Ex: char szMess[] = "Voici un \x01."  affichera Voici un �.
 *
 *               NOTE: Pour afficher un �, on devra prendre LcdEcritCar() puisque \x00 est une fin de cha�ne.
 *               										   -
 *                  INT ==> Intensite ajust�e via un Potentiometre 10K
 *
 *               L'affichage utilise 7 lignes pour fonctionner.
 *
 *               4 sont utilis�es pour les donn�es (ON UTILISE NORMALEMENT 8)
 *               1 est utilis�e pour un CE
 *               1 est utilis�e comme R/-W
 *               1 est utilis�e comme Register Select
 */

#include "Lcd4Lignes.h"

#define RS          PORTAbits.RA0 
#define RW          PORTAbits.RA1 
#define EN          PORTAbits.RA2 
#define MODE_IN     TRISD = 0b00001111 //TRISA = 0b00001111
#define MODE_OUT    TRISD = 0b00000000

#define DATA_8_BITS         0x03            // Pour initialisation
#define DATA_4_BITS         0x02            // Pour initialisation
#define INIT_DISPLAY        0x28            // 4 bits, 2 lignes, 5x7 ==> 2 �critures

// Instructions pour le LCD
#define VIDE_ECRAN          0x01            // Display Clear
#define HOME_CURSEUR        0x02            // Cursor Home
#define CURSEUR_MOVE        0x06            // Entry Mode Set
#define CURSEUR_OFF         0x0C         
#define CURSEUR_ON          0x0F            
#define CURSEUR_LEFT        0x10
#define DISPLAY_LEFT        0x18
#define SET_CGRAM_ADR       0x40
#define SET_RAM_ADR         0x80    

#define LIGNE1_START        0x80            // Adresses de d�but des lignes
#define LIGNE2_START        0xC0
#define LIGNE3_START        0x94
#define LIGNE4_START        0xD4



// D�finition des variables globales

// Variable pour la g�n�rations de certains caract�res en CGRAM
// On aura dans l'ordrer (8 octets par caract�re): � � � � � � � �

char matCGRAM[8][8] =
{
    {8,4,4,0x0A,0x11,0x1F,0x11,0},            // �
    {0x1F, 0x11, 0x15, 0x15, 0x15, 0x15, 0x11, 0x1F},  // Tuile
    {0x02, 0x04, 0x04, 0x0E, 0x1F, 0x1F, 0x1F, 0x0E},  // Bombe    
    {0x10, 0x1B, 0x1E, 0x1F, 0x14, 0x10, 0x10, 0x1F},        // Drapeau
    {8,4,0x1F,0x10,0x1E,0x10,0x1F,0},        // �
    {4,0x0A,0x0E,0x11,0x1F,0x10,0x0E,0},    // �
    {8,4,0x0E,01,0x0F,0x11,0x0F,0},            // �    
    {4,0x0A,0x0E,1,0x0F,0x11,0x0F,0},        // �
};

// Prototypes des fonctions internes

static unsigned char lcd_busy(void);
static unsigned char lcd_wrCom(unsigned char cCommande);
static unsigned char lcd_wrData(unsigned char nCaractere);
static unsigned char lcd_lireDonnees(void);
static void lcd_ecrireDonnees(unsigned char cDonnee);
static void lcd_resetSequence(void);
static void lcd_initCGRam(void);
static void lcd_strobeEnableBit(void);

// Cette fonction permettra de lire le port de donn�es que
// l'utilisateur a indiqu� et reconfigurer tristX pour une lecture
// de 4 des bits du port car on devra faire 2 lecture pour le 8 bits
// sans changer les 4 autres bit du trisX concern�
static unsigned char lcd_lireDonnees(void)
{
    //return (PORTD >> 4);
    return (PORTD & 0x0f);
    //return (PORTA & 0x0F);
}

// Cette fonction permettra d'�crire le port de donn�es que
// l'utilisateur a indiqu� et reconfigurer tristX pour une lecture
// sans changer les 4 autres bit du trisX concerne
// On devra �crire les 4 bits ind�pendemments car on ne sait pas ce que
// l'utilisateur a fait avec les 4 autres lignes du port. On ne 
// peut pas assumer que ce sont des E/S

static void lcd_ecrireDonnees(unsigned char donnee)
{
    //PORTD = (donnee<<4) | (PORTD & 0x0F);	//NE PAS modifier les bits D3..D0
    PORTD = (donnee & 0x0F) | (PORTD & 0xF0);
    /*PORTAbits.RA0 = (donnee & 0x01);
    PORTAbits.RA1 = ((donnee >> 1) & 0x01);
    PORTAbits.RA2 = ((donnee >> 2) & 0x01);
    PORTAbits.RA3 = ((donnee >> 3) & 0x01);*/
}

/**
 *   Entree:     Rien
 *   Sortie:     Retourne l'adresse du curseur
 *   Utilite:    Fonction qui v�rifie si l'affichage est pr�t � recevoir une commande
 *               ou un caract�re.
 */

static unsigned char lcd_busy(void)
{
    unsigned char adresse;
    
    RS = 0;                 
    RW = 1;                         

    do
    {
        EN = 1;                    // ENABLE = 1
        NOP();
        adresse = lcd_lireDonnees() << 4;              // Lire le premier 4 bits (D4->D7)
        EN = 0;                     // ENABLE = 0
        __delay_us(2);
        EN = 1;                    // ENABLE = 1
        NOP();
        adresse |= lcd_lireDonnees();                  // Lire le dernier 4 bits (D0->D3)
        EN = 0;                     // ENABLE = 0
        __delay_us(2);
    }
    while(adresse&0x80);

    RW = 0;
    return(adresse&0x7f);
}

/*
    Entree:     Commande � ecrire
    Sortie:     Adresse du curseur
    Utilite:    Fonction pour �crire une commande dans l'affichage
*/

static unsigned char lcd_wrCom(unsigned char commande)
{
    lcd_busy();

    MODE_OUT;
    __delay_us(2);
     
    RS = 0;                 // RS = 0
    RW = 0;                 // RW = 0
    
    lcd_ecrireDonnees(commande >> 4);      // Les 4 bits sup�rieurs
    lcd_strobeEnableBit();
    __delay_us(2);

    lcd_ecrireDonnees(commande);           // Les 4 bits inf�rieurs
    lcd_strobeEnableBit();
    __delay_us(40);     	// Temps Max d'ex�cution d'une commande

    if ((commande == VIDE_ECRAN) || commande == HOME_CURSEUR)
        __delay_ms(2);

    MODE_IN;
    __delay_us(2);
    return(lcd_busy());
}

/*
    Entree:     Caract�re � �crire
    Sortie:     Adresse du curseur
    Utilite:    Fonction pour �crire un caract�re dans l'affichage
*/
 
static unsigned char lcd_wrData(unsigned char caractere)
{
    lcd_busy();                             // V�rifie si affichage pr�t � recevoir  le caract�re
    MODE_OUT;
    __delay_us(2);

    RS = 1;                // RS = 1
    RW = 0;                 // RW = 0

    lcd_ecrireDonnees(caractere >> 4);     // Les 4 bits sup�rieurs
    lcd_strobeEnableBit();
    __delay_us(2);

    lcd_ecrireDonnees(caractere);          // Les 4 bits inf�rieurs
    lcd_strobeEnableBit();
    __delay_us(2);

    MODE_IN;
    __delay_us(2);
    return(lcd_busy());
}

/*
    Entree:     Rien
    Sortie:     Rien
    Utilite:    Fonction pour initialiser l'affichage en mode 4 bits
                C'est cette fonction qui placera l'affichage en mode 4 bits
                pour ensuite pouvoir �crire des 8 bits par bloc de 4 bits
                en commencant par la partie haute du 8 bits.
*/                                                



static void lcd_resetSequence(void)
{
    // Il faut transmettre la s�quence telle que d�crite dans la fiche
    // signalitique de Seiko

    MODE_OUT;

    __delay_ms(40);
    RS = 0;                 // RS = 0
    RW = 0;                 // RW = 0

    lcd_ecrireDonnees(0x03);
    lcd_strobeEnableBit();
    __delay_us(40);
    

    lcd_ecrireDonnees(0x03);
    lcd_strobeEnableBit();
    __delay_us(40);
    
    lcd_ecrireDonnees(0x03);
    lcd_strobeEnableBit();
    __delay_us(40);
    
    lcd_ecrireDonnees(0x02);
    lcd_strobeEnableBit();
    __delay_us(40);

    MODE_IN;
}

/*
    Entree:     Rien
    Sortie:     Rien
    Utilite:    Fonction pour initialiser complete de l'affichage
*/                                                

void lcd_init(void)
{    
    //TRISB = TRISB & 0xEF;       // B4 en out
    lcd_resetSequence();            // S�quence pour mettre en 4 bits
    lcd_wrCom(INIT_DISPLAY);        // System set: 4 bits, 2 lignes, 5x7
    lcd_wrCom(CURSEUR_ON);          // Display, curseur et blink on
    lcd_wrCom(VIDE_ECRAN);          // Efface �cran et initialise curseur
    lcd_wrCom(CURSEUR_MOVE);        // Position du curseur incr�mente sans scroll
    lcd_wrCom(SET_RAM_ADR);         // Adresse de la position d'�criture a zero
    lcd_initCGRam();
}

/*
    Entree:     Rien
    Sortie:     Rien
    Utilite:    Fonction pour effacer l'affichage LCD
*/

void lcd_effaceAffichage(void)
{
    lcd_wrCom(VIDE_ECRAN);          // Vide l'affichage
    lcd_wrCom(SET_RAM_ADR);         // Adresse de la position d'�criture a zero
} 

/*
    Entree:     Rien
    Sortie:     Rien
    Utilite:    Fonction pour ramener le curseur a la ligne 1, colonne 1
*/

void lcd_curseurHome(void)
{
    lcd_wrCom(HOME_CURSEUR);      // Transmet la commande
}

/*
    Entree:     Nombre de caracteres a effacer
    Sortie:     Rien
    Utilite:    Fonction pour effacer un nombre de caract�res puis repositionner 
                le curseur
*/

void lcd_effaceChar(unsigned char nbr)
{
    char i;
    for(i=0;i<nbr;i++)
        lcd_wrData(' ');            // Efface un caract�re et conserve adresse
}

/*
    Entree:     Position en X (1->20) et en Y (1 ->4)
    Sortie:     Rien
    Utilite:    Fonction pour positionner le curseur a une position dans l'affichage
                On peut utiliser 4 lignes dont les adresses sont 0, 0x40 0x14 et 0x54
*/

void lcd_gotoXY(unsigned char x, unsigned char y)
{
    switch(y)
    {
        case 1:
            lcd_wrCom((x-1+LIGNE1_START)|SET_RAM_ADR);
            break;
        case 2:
            lcd_wrCom((x-1+LIGNE2_START)|SET_RAM_ADR);
            break;
        case 3:
            lcd_wrCom((x-1+LIGNE3_START)|SET_RAM_ADR);
            break;
        case 4:
            lcd_wrCom((x-1+LIGNE4_START)|SET_RAM_ADR);
            break;
    }
}   

/*
    Entree:     Rien
    Sortie:     Rien
    Utilite:    Permet d'effacer la premi�re ligne de l'affichage lcd
*/

void lcd_effaceLigne(unsigned char y)
{
    lcd_gotoXY(1,y);
    lcd_effaceChar(20);
    lcd_gotoXY(1,y);
}


/*
    Entree:     Le caractere � afficher
    Sortie:     Rien
    Utilite:    Permet d'afficher un caract�re � la position du cursuer
                Va aussi s'assurer que si le curseur se retrouve � une position
                qui ferait un changement de ligne, alors on ajustera pour aller
                � la ligne suivante sur l'affichage

                Ex: Si on passe de l'adresse 0x13 -> 0x14 alors on passe de la ligne 1 -> 3
                    On va donc changer le 0x14 pour 0x40 et on passera de la ligne   1 -> 2
*/

void lcd_ecritChar(unsigned char car)
{
    unsigned char posRam;

    posRam = lcd_wrData(car);
    switch(posRam)
    {
        case LIGNE3_START:                        // Curseur de fin ligne 1 � d�but ligne 3
            lcd_wrCom(LIGNE2_START|SET_RAM_ADR);    // Remettre au d�but de la seconde
            break;
        case LIGNE4_START:                        
            lcd_wrCom(LIGNE3_START|SET_RAM_ADR);
            break;
        case LIGNE2_START:                    
            lcd_wrCom(LIGNE4_START|SET_RAM_ADR);
            break;
    }
 }


/*
    Entree:     Adresse du message � afficher
    Sortie:     Rien
    Utilite:    Fonction pour afficher une chaine a la position du curseur
                Param�tre d'appel: adresse d'une chaine non constante
                On va traiter le changement de lignes ad�quatement pour aller sur la bonne ligne
*/

void lcd_putMessage(const unsigned char *chaine)
{
    unsigned char j;
   
    for(j = 0; chaine[j] != 0; j++)
        lcd_ecritChar(chaine[j]);
}

/*
    Entree:     Rien
    Sortie:     Rien
    Utilite:    Permet de cacher le curseur.
*/

void lcd_cacheCurseur(void)
{
    lcd_wrCom(CURSEUR_OFF);
}

/*
    Entree:     Rien
    Sortie:     Rien
    Utilite:    Permet de montrer le curseur.
*/

void lcd_montreCurseur(void)
{
    lcd_wrCom(CURSEUR_ON);
}

static void lcd_initCGRam(void)        // pMatCGRAM
{
    char i,j;
    
    lcd_wrCom(SET_CGRAM_ADR);
    for(i=0;i<8;i++)
    {
        for(j=0;j<8;j++)
            lcd_wrData(matCGRAM[i][j]);
    }
    lcd_curseurHome();
}


static void lcd_strobeEnableBit(void)
{
    EN = 1;            // ENABLE = 1
    NOP();
    EN = 0;             // ENABLE = 0
}