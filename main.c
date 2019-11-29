 /**
 * @file   main.c  Lab_6_AA_Demineur
 * @author Alexandre Alain
 * @date   XXXXX Novembre 2019
 * @brief  Jeux de D�mineur refait � la TSO. Le joueur 
 *
 * @version 1.0
 * Environnement:
 *     D�veloppement: MPLAB X IDE (version 5.05)
 *     Compilateur: XC8 (version 2.10)
 *     Mat�riel: Carte d�mo du Pickit3. PIC 18F45K20
  */

/****************** Liste des INCLUDES ****************************************/
#include <xc.h>
#include <stdbool.h>  // pour l'utilisation du type bool
#include <conio.h>
#include "Lcd4Lignes.h"
#include "serie.h"
#include <stdio.h> //pour rand()
#include <stdlib.h> //pour rand()
#include <string.h> //pour strcpy()



/********************** CONSTANTES *******************************************/
#define NB_LIGNE 4  //afficheur LCD 4x20
#define NB_COL 20
#define AXE_X 7  //canal analogique de l'axe x
#define AXE_Y 6
#define PORT_SW PORTBbits.RB1 //sw de la manette
#define TUILE 1 //caract�re cgram d'une tuile
#define MINE 2 //caract�re cgram d'une mine




/********************** PROTOTYPES *******************************************/
void initialisation(void);
char getAnalog(char canal);
void lcd_init(void);
void init_serie(void);
void lcd_curseurHome(void);
void lcd_effaceAffichage(void);
void lcd_putMessage(const unsigned char *chaine);
void lcd_gotoXY(unsigned char x, unsigned char y);
void lcd_ecritChar(unsigned char car);
int stricmp(const char *string1, const char *string2);

void deplace(char* x);
void feu(char x, int* pts);
bool perdu(void);
void affichePerdu(int pts);
char *strcpy(char *dest, const char *src);
bool testEtat(void);

void initTabVue(void);
void rempliMines(int nb);
void metToucheCombien(void);
char calculToucheCombien(int ligne, int colonne);
void deplace(char* x, char* y);
bool demine(char x, char y);
void enleveTuilesAutour(char x, char y);
bool gagne(int* pMines);



/****************** VARIABLES GLOBALES ****************************************/
char m_tabMines[NB_LIGNE][NB_COL+1]; //Tableau des caract�res affich�s au LCD
char m_tabVue[NB_LIGNE][NB_COL+1]; //Tableau contenant les mines, les espaces et les chiffres

/*               ***** PROGRAMME PRINCPAL *****                             */
void main(void)
{
    /*********variables locales du main********/
    const char afficheNom[] = "Lab6 Alexandre Alain";  
    int posX = 10; //posisition du joueur, initialement � 10 (milieu)
    int pointage = 0; //pour compter le nombre de points
    bool etatInitSW = false; //servira � �viter que le joueur laisse le bouton appuy�
    bool etatAfterSW = false;
    
    /*********d�but du code********************/
    initialisation(); //initialise les entr�s/sorties
    lcd_init();
    init_serie();    
    lcd_cacheCurseur(); //enl�ve le curseur car le flash "gossait"    
    lcd_gotoXY(1, 1);    
    lcd_putMessage(afficheNom); //affiche nom et labo avant de commencer
    
    __delay_ms(2000);
        
    etatInitSW = testEtat(); //mets l'�tat de la swicth avant/apr�s � jour
    etatAfterSW = etatInitSW;
    
    while(1) //boucle principale
    {        
        
        if(m_tempsDAfficher == true)//si temps �coul�, selon valeur entr�e dans les defines
        {
            nouveauxAliens(); //fait appara�tre les aliens dans la matrice
            
            if(perdu() == true) //regarde si le joueur a perdu la partie
            {
                affichePerdu(pointage); //affiche message de game over et le pointage final
                posX = 10;//remet le joueur eu milieu
                videAliens(); // vide le matrice pour recommencer la partie � neuf
                pointage = 0; //reset le score � 0
            }
            
            afficheAliens(); //affiche la matrice(aliens) sur le LCD
            lcd_gotoXY(posX, 4);
            m_tempsDAfficher = false; //remets a false pour un nouveau timer
        }
        
        deplace(&posX); //d�place le joueur selon le sens voulu
        
        etatInitSW = testEtat(); //regarde l'�tat de la switch
        
        if( (etatInitSW != etatAfterSW) && (etatAfterSW == false) ) //si SW manette apuy�, trouv� c'est lequel
        {
            feu(posX, &pointage);
        }
        
        etatAfterSW = etatInitSW; //enregistre le dernier �tat
        __delay_ms(100);
    }
}


/*
 * @brief Rempli le tableau m_tabVue avec le caract�re sp�cial (d�finie en CGRAM
 *  du LCD) TUILE. Met un '\0' � la fin de chaque ligne pour faciliter affichage
 *  avec lcd_putMessage().
 * @param rien
 * @return rien
 */
void initTabVue(void)
{
    
}
 
/*
 * @brief Rempli le tableau m_tabMines d'un nombre (nb) de mines au hasard.
 *  Les cases vides contiendront le code ascii d'un espace et les cases avec
 *  mine contiendront le caract�re MINE d�fini en CGRAM.
 * @param int nb, le nombre de mines � mettre dans le tableau 
 * @return rien
 */
void rempliMines(int nb)
{
    
}
 
/*
 * @brief Rempli le tableau m_tabMines avec le nombre de mines que touche la case.
 * Si une case touche � 3 mines, alors la m�thode place le code ascii de 3 dans
 * le tableau. Si la case ne touche � aucune mine, la m�thode met le code
 * ascii d'un espace.
 * Cette m�thode utilise calculToucheCombien(). 
 * @param rien
 * @return rien
 */
void metToucheCombien(void)
{
    
}
 
/*
 * @brief Calcul � combien de mines touche la case.
 * @param int ligne, int colonne La position dans le tableau m_tabMines a v�rifier
 * @return char nombre. Le nombre de mines touch�es par la case
 */
char calculToucheCombien(int ligne, int colonne)
{
    
}
 
/**
 * @brief Si la manette est vers la droite ou la gauche, on d�place le curseur 
 * d'une position (gauche, droite, bas et haut)
 * @param char* x, char* y Les positions X et y  sur l'afficheur
 * @return rien
 */
void deplace(char* x, char* y)
{
    
}
 
/*
 * @brief D�voile une tuile (case) de m_tabVue. 
 * S'il y a une mine, retourne Faux. Sinon remplace la case et les cases autour
 * par ce qu'il y a derri�re les tuiles (m_tabMines).
 * Utilise enleveTuileAutour().
 * @param char x, char y Les positions X et y sur l'afficheur LCD
 * @return faux s'il y avait une mine, vrai sinon
 */
bool demine(char x, char y)
{
    
}
 
/*
 * @brief D�voile les cases non min�es autour de la tuile re�ue en param�tre.
 * Cette m�thode est appel�e par demine().
 * @param char x, char y Les positions X et y sur l'afficheur LCD.
 * @return rien
 */
void enleveTuilesAutour(char x, char y)
{
    
}
 
/*
 * @brief V�rifie si gagn�. On a gagn� quand le nombre de tuiles non d�voil�es
 * est �gal au nombre de mines. On augmente de 1 le nombre de mines si on a 
 * gagn�.
 * @param int* pMines. Le nombre de mine.
 * @return vrai si gagn�, faux sinon
 */
bool gagne(int* pMines)
{
    
}

















/*
 * @brief Renvoie l'�tat actuel de la SW par true ou false
 * @param rien
 * @return bool
 */
bool testEtat(void)
{
    if(PORTBbits.RB1 == 0) //si SW manette appuy�e retourne true
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*
 * @brief D�cale les lignes vers le bas et g�n�re une nouvelle ligne avec NB_ALIENS aliens et m_niveau
 * Note: Cette m�thode n'affiche rien sur l'afficheur LCD. Elle ne travaille qu'avec la matrice m_aliens[][]
 * @param rien
 * @return rien
 */
void nouveauxAliens()
{
    int posAlien[NB_COL]; //tableau pour g�rer si place d�j� occup� ou non
    int verifPosition = 0;
    int i = 0;
    int monstre = 0;
    
    //recopie la ligne une ligne plus bas en partant de la plus basse
    strcpy(m_aliens[3], m_aliens[2]);
    strcpy(m_aliens[2], m_aliens[1]);
    strcpy(m_aliens[1], m_aliens[0]);
    
    //mets la nouvelle ligne et la v�rif. � 0 pour afficher nouveaux aliens
    for(int j = 0 ; j < NB_COL ; j++)
    {
        posAlien[j] = 0;
        m_aliens[0][j] = ' ';
    }
    
    //ajoute des aliens sur la ligne/matrice jusqu'au nombre en cours selon le niveau
    while(i < (NB_ALIENS + m_niveau))
    {
        verifPosition = rand() % 20;
        if(posAlien[verifPosition] == 0)
        {
            monstre = rand()%3+1; //random pour le monstre � afficher
            m_aliens[0][verifPosition] = monstre; //mets le monstre dans la matrice et sur le LCD
            posAlien[verifPosition] = 1;
            i++;
        }
    }    
}
 
/*
 * @brief Affiche les 4 lignes de la matrice m_aliens[][] sur l'afficheur LCD.
 * @param rien
 * @return rien
 */
void afficheAliens()
{
    lcd_effaceAffichage(); //efface le tout avant d'afficher les nouvelles lignes
    
    for(int i = 0 ; i < 4 ; i++) //affiche les 4 lignes de la matrice aliens sur la LCD
    {
        lcd_gotoXY(1, i+1);
        lcd_putMessage(m_aliens[i]);
    }
    
}

/*
 * @brief Efface jeu et vide matrice des aliens pour une nouvelle partie.
 * @param rien
 * @return rien
 */
void videAliens()
{
    lcd_effaceAffichage();
    //remets toute la matrice � ' ' (vide)
    for(int i =0 ; i < NB_LIGNE ; i++)
    {
        for(int j = 0 ; j < NB_COL ; j++)
        {        
            m_aliens[i][j] = ' ';
        }
    }    
    
}
 
/**
 * @brief Si la manette est vers la droite ou la gauche, on d�place le curseur d'une position
 * @param La position X sur la ligne (i.e. le pointeur de posX du main).
 * @return rien
 */
void deplace(char* x)
{
    int tension; //pour enregistrer la valeur binaire lu
    //pour enlever la derni�re position du joueur
    const char effaceJoueur[21] = {"                    "};
    
    tension = getAnalog(7); //capture la tension
        
    //regarde la tension du du joystick, si < ou > on posX ++ ou --
    if(tension > 200)
    {
        *x = *x + 1;
    }
    else if(tension < 50)
    {
        *x = *x - 1;
    }
    
    //pour qd d�passe les limites
    if(*x == 21)
        *x = 1;
    if(*x == 0)
        *x = 20;
    
    //efface derni�re position et place nouvelle position du joueur
    lcd_gotoXY(1, 4);
    lcd_putMessage(effaceJoueur);
    lcd_gotoXY(*x, 4);
    lcd_ecritChar('A');
}
 
/*
 * @brief S'il y a un alien en haut (ligne 0,1 ou 2 en position X), on le tue
 * @param La position X sur la ligne. Le pointeur vers le nombre de points (le nombre d'aliens morts).
 * @return rien
 */
void feu(char x, int* pts)
{
    for(int i = 2 ; i>= 0 ; i--) //part du bas vers la haut
    {
        if(m_aliens[i][x-1] != ' ') //v�rifie s'il y a un alien devant
        {
            //incr�mente les pts selon quel alien a �t� tu�
            if(m_aliens[i][x-1] == 1)
            {
                *pts = *pts + 1;
            }
            if(m_aliens[i][x-1] == 2)
            {
                *pts = *pts + 2;
            }
            if(m_aliens[i][x-1] == 3)
            {
                *pts = *pts + 3;
            }
            m_aliens[i][x-1] = ' '; //enl�ve l'alien de la matrice
            lcd_gotoXY(x, i+1);
            lcd_ecritChar(' '); //efface tout de suite l'alien du LCD
            
            break;
        }
    }    
}
 
/*
 * @brief Lorsqu'on arrive ici, le temps d'affichage d'un �cran est �coul�.
 *        Donc, s'il y a au moins 1 aliens sur la ligne 3, on a perdu
 * @param rien
 * @return vrai si on a perdu, faux sinon
 */
bool perdu(void)
{
    //tableau vide pour savoir si le joueur a tu� tous les aliens
    const char safe[21] = {"                    "};
    
    //compare la ligne d'alien � la ligne safe pour savoir si perdu
    if(strcmp(safe, m_aliens[3]) == 0)
    {
        return false;
    }
    else
    {
        return true;
    }    
}
 
 
/*
 * @brief Affiche le nombre de points obtenus pendant 3 secondes.
 * @param int pts. Les points accumul�s
 * @return rien
 */
void affichePerdu(int pts)
{
    const char gameOver[] = "LOL you lost"; //pcq juste dire perdu c'est pas assez ^^    
    char ptsChar[3];
    //Cr�e une string en incluant le score final pour le lcd_putMessage
    sprintf(ptsChar, "Score = %3d", pts);
    
    //affiche le message (pas gentil) de game over et pointage final
    lcd_effaceAffichage();
    lcd_gotoXY(4, 1);
    lcd_putMessage(gameOver);
    lcd_gotoXY(4, 3);
    lcd_putMessage(ptsChar);
    __delay_ms(2000);
    lcd_effaceAffichage();
    
    //si plus de 10 pts, prochaine partie monte le niveau de difficult�
    //et ajoute un alien
    if(pts > 10)
    {
        m_niveau++;
        if(m_niveau> (20-NB_ALIENS) ) //pour avoir max 20 aliens en jeu
        {
            m_niveau = (20-NB_ALIENS);
        }
    }    
}



/*
 * @brief Lit le port analogique. 
 * @param Le no du port � lire
 * @return La valeur des 8 bits de poids forts du port analogique
 */
char getAnalog(char canal)
{ 
    ADCON0bits.CHS = canal;
    __delay_us(1);  
    ADCON0bits.GO_DONE = 1;  //lance une conversion
    while (ADCON0bits.GO_DONE == 1); //attend fin de la conversion
    return  ADRESH; //retourne seulement les 8 MSB. On laisse tomber les 2 LSB de ADRESL
}
   
/*
 * @brief Fait l'initialisation des diff�rents regesitres et variables.
 * @param Aucun
 * @return Aucun
 */
void initialisation(void)
{
    TRISD = 0; //Tout le port D en sortie
 
    ANSELH = 0;  // RB0 � RB4 en mode digital. Sur 18F45K20 AN et PortB sont sur les memes broches
    TRISB = 0xFF; //tout le port B en entree
 
    ANSEL = 0;  // PORTA en mode digital. Sur 18F45K20 AN et PortA sont sur les memes broches
    TRISA = 0; //tout le port A en sortie
 
    //Pour du vrai hasard, on doit rajouter ces lignes. 
    //Ne fonctionne pas en mode simulateur.
    T1CONbits.TMR1ON = 1;
    srand(TMR1);
 
   //Configuration du port analogique
    ANSELbits.ANS7 = 1;  //A7 en mode analogique
 
    ADCON0bits.ADON = 1; //Convertisseur AN � on
	ADCON1 = 0; //Vref+ = VDD et Vref- = VSS
 
    ADCON2bits.ADFM = 0; //Alignement � gauche des 10bits de la conversion (8 MSB dans ADRESH, 2 LSB � gauche dans ADRESL)
    ADCON2bits.ACQT = 0;//7; //20 TAD (on laisse le max de temps au Chold du convertisseur AN pour se charger)
    ADCON2bits.ADCS = 0;//6; //Fosc/64 (Fr�quence pour la conversion la plus longue possible)
 
 
    /**************Timer 0*****************/
    T0CONbits.TMR0ON    = 1;
    T0CONbits.T08BIT    = 0; // mode 16 bits
    T0CONbits.T0CS      = 0;
    T0CONbits.PSA       = 0; // prescaler enabled
    T0CONbits.T0PS      = 0b010; // 1:8 pre-scaler
    TMR0 = DELAI_TMR0;
    INTCONbits.TMR0IE   = 1;  // timer 0 interrupt enable
    INTCONbits.TMR0IF   = 0; // timer 0 interrupt flag
    INTCONbits.PEIE = 1; //permet interruption des p�riph�riques
    INTCONbits.GIE = 1;  //interruptions globales permises
}
 
/*
 * @brief Interruptions du PIC. On trouve d'o� elle vient (ici TMR0), on recharge
 * le registre pour la prochaine et on idique au main que le temps est �coul� via 
 * la variable m_tempsDAfficher
 * @param Aucun
 * @return Aucun
 */
void interrupt high_isr(void)
{
    if (INTCONbits.TMR0IF) // timer 0
    {
        INTCONbits.TMR0IF   = 0;    // baisser l'indication de l'interruption
 
        TMR0 = DELAI_TMR0;  //recharge pour la prochaine interruption
        m_tempsDAfficher = true; //Indique au main que le temps est �coul�
    }
}
