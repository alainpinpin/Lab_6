 /**
 * @file   main.c  Lab_6_AA_Demineur
 * @author Alexandre Alain
 * @date   6 D�cembre 2019
 * @brief  Jeux de D�mineur refait � la TSO. Le num�ro du laboratoire et le nom du programmeur 
  * s'affiche � l'�cran sur un cours laps de temps et ensuite le jeu commence. L'�cran est remplie 
  * de tuiles. La position du joueur <<flash>>, il se d�place gr�ce au <<joystick>>. Si le joueur 
  * appuie sur l'interrupteur du <<joystick>> cela d�mine la case et les vides autour.
  * Si la case �tait une mine alors la partie est perdu. L'�cran affiche alors les mines et les chiffres.
  * Le joueur peut recomencer une nouvelle partie en appuyant de nouveau sur la manette.
  * Au d�but il y a 5 mines de cach�es, le nombre augmente de un � chaque partie gagn�e.
  * La partie est gagn�e si toutes les cases, sauf les mines, sont d�min�es. Les mines 
  * ils apparaissent et on appuie pour une nouvelle partie.
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
#define _XTAL_FREQ 1000000 //Constante utilis�e par __delay_ms(x). Doit = fr�q interne du uC
#define NB_LIGNE 4  //afficheur LCD 4x20
#define NB_COL 20
#define AXE_X 7  //canal analogique de l'axe x
#define AXE_Y 6
#define PORT_SW PORTBbits.RB1 //sw de la manette
#define TUILE 1 //caract�re cgram d'une tuile
#define MINE 2 //caract�re cgram d'une mine
#define DRAPEAU 3 //caract�re cgram d'un drapeau
#define STARTINGBOMBS 5 //Changer ici pour le nombre initiale de bombes


/********************** PROTOTYPES *******************************************/
void initialisation(void);
char getAnalog(char canal);
void lcd_init(void);
void init_serie(void);
void lcd_curseurHome(void);
void lcd_effaceAffichage(void);
void lcd_montreCurseur(void);
void lcd_putMessage(const unsigned char *chaine);
void lcd_gotoXY(unsigned char x, unsigned char y);
void lcd_ecritChar(unsigned char car);
char *strcpy(char *dest, const char *src);


bool testEtat(void);
void initTabVue(void);
void rempliMines(int nb);
void metToucheCombien(void);
void afficheTabMines(void);
void afficheTabVue(void);
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
    int posX = 10; //posisition du joueur, initialement � 10/3
    int posY = 3;     
    bool etatInitSW = false; //servira � �viter que le joueur laisse le bouton appuy�
    bool etatAfterSW = false;
    int nbMines = STARTINGBOMBS;
    
    /*********d�but du code********************/
    initialisation(); //initialise les entr�s/sorties
    lcd_init();    
       
    etatInitSW = testEtat(); //mets l'�tat de la swicth avant/apr�s � jour
    etatAfterSW = etatInitSW;
    
    lcd_gotoXY(1, 1);    
    lcd_putMessage(afficheNom); //affiche nom et labo avant de commencer
    
    __delay_ms(2000);
        
    lcd_effaceAffichage();
    
    
    //remplie nos 2 tableaux et fait afficher les tuiles
    initTabVue();
    rempliMines(nbMines);
    metToucheCombien();
    afficheTabVue();
    
    while(1) //boucle principale
    {        
        
        deplace(&posX, &posY); //d�place le joueur (duh...)
                
        etatInitSW = testEtat(); //regarde l'�tat de la switch
        
        if( (etatInitSW != etatAfterSW) && (etatAfterSW == false) ) //si SW manette apuy�, trouv� c'est lequel
        {
                   
            if( (demine(posX, posY) == false) || ( gagne(&nbMines) == true) )
            {
                lcd_cacheCurseur(); //par soucie d'esth�tique
                afficheTabMines(); //montre tableau avec mines et chiffres
                __delay_ms(500);//d�lai pour eviter de restarter trop vite
                
                while(testEtat() != true) //attend que le joueur appuie pour recommencer
                {
                    __delay_ms(100);
                }
                
                lcd_montreCurseur();
                initTabVue();
                rempliMines(nbMines);
                metToucheCombien();
            }
            
            afficheTabVue();
        }
        
        etatAfterSW = etatInitSW; //enregistre le dernier �tat
        
        /*Bonus du drapeau : fait apparaitre un drapeau flottant au vent si on appuie sur la SW1
        de la carte alors que le joueur se trouve sur une tuile. Remets une tuile si 
        il y a d�j� un drapeau. */
        if (PORTBbits.RB0 == 0)
        {
            if(m_tabVue[posY-1][posX-1] == TUILE)
            {
                m_tabVue[posY-1][posX-1] = DRAPEAU;
            }
            else if(m_tabVue[posY-1][posX-1] == DRAPEAU)
            {
                m_tabVue[posY-1][posX-1] = TUILE;
            }
            
            afficheTabVue();
            
            while (PORTBbits.RB0 == 0);//on attend que le bouton soit rel�ch�
            
        }
              
        __delay_ms(100);
        
    }
}

/*
 * @brief Affiche sur le LCD notre matrice m_tabMines.
 * @param rien
 * @return rien
 */
void afficheTabMines(void)
{
    lcd_effaceAffichage();
    for(int k = 0 ; k < 4 ; k++)
    {
        lcd_gotoXY(1, (k+1) );
        lcd_putMessage(m_tabMines[k]);
    }
}

/*
 * @brief Affiche sur le LCD notre matrice m_tabVue.
 * @param rien
 * @return rien
 */
void afficheTabVue(void)
{
    lcd_effaceAffichage();
    for(int k = 0 ; k < 4 ; k++)
    {
        lcd_gotoXY(1, (k+1) );
        lcd_putMessage(m_tabVue[k]);
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
    for(int i = 0 ; i < NB_LIGNE ; i++)
    {
        for(int k = 0 ; k < NB_COL ; k++)
        {
            m_tabVue[i][k] = TUILE;
        }
    }
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
    bool tabSafe[NB_LIGNE][NB_COL]; //matrice pour s'assurer que 2 bombe ne se "stack" pas
    char testX = 0;
    char testY = 0;
    
    //mets la tableau de safety a false et mets les espaces dans tabMines 
    for(int j = 0 ; j < NB_COL ; j++)
    {
        for(int m = 0; m < NB_LIGNE ; m++)
        {
            tabSafe[m][j] = false;
            m_tabMines[m][j] = ' ';
        }
    }
          
    //mets les mines dans la matrice m_tabMines al�atoirement
    for(int i = 0 ; i < nb ; i++)
    {
        do
        {            
            testX = rand()%20;
            testY = rand()%4;
        }
        while(tabSafe[testY][testX] == true);// s'assure que le case n'est pas d�j� occup�e

        tabSafe[testY][testX] = true;//mets la case � true pour �viter de r��crire par dessus
        m_tabMines[testY][testX] = MINE; //mets une mine � l'endroit vide
    }  
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
    char chiffre = 0; // chiffre � afficher
    
    for(int j = 0 ; j < NB_COL ; j++)
    {
        for(int m = 0; m < NB_LIGNE ; m++)
        {
            chiffre = calculToucheCombien(m, j);
                        
            if( chiffre != 0 && m_tabMines[m][j] != MINE) //si case diff�rente d'une mine, met le chiffre
            {
                m_tabMines[m][j] = 48 + chiffre;
            }
             
        }
    }    
}
 
/*
 * @brief Calcul � combien de mines touche la case.
 * @param int ligne, int colonne La position dans le tableau m_tabMines a v�rifier
 * @return char nombre. Le nombre de mines touch�es par la case
 */
char calculToucheCombien(int ligne, int colonne)
{
    unsigned char nombre = 0;
    signed char minLigne = -1;
    signed char maxLigne = 2;
    signed char minCol = -1;
    signed char maxCol = 2;
    
    //g�re les diff�rents cas limites sur les bords
    if(ligne == 0)
        minLigne++;
    if(ligne == 3)
        maxLigne--;
    if(colonne == 0)
        minCol++;
    if(colonne == 19)
        maxCol--;
    
    
    for(signed char j = minCol ; j < maxCol ; j++)
    {
        for(signed char m = minLigne; m < maxLigne ; m++)
        {            
            if( m_tabMines[ligne+m][colonne+j] == MINE)
            {
                nombre++; //incr�mente le nombre si une mine est autour
            }
        }
    }    
    
    return nombre;
}
 
/**
 * @brief Si la manette est vers la droite ou la gauche, on d�place le curseur 
 * d'une position (gauche, droite, bas et haut)
 * @param char* x, char* y Les positions X et y  sur l'afficheur
 * @return rien
 */
void deplace(char* x, char* y)
{
    
    int tensionX; //pour enregistrer la valeur binaire lu pour l'axe des x
    int tensionY; //pour enregistrer la valeur binaire lu pour l'axe des y
            
    tensionX = getAnalog(AXE_X); //capture la tension pour x
    tensionY = getAnalog(AXE_Y); //capture la tension pour y
        
    //regarde la tension du du joystick, si < ou > on posX ++ ou --
    if(tensionX > 200)
    {
        *x = *x + 1;
    }
    else if(tensionX < 50)
    {
        *x = *x - 1;
    }
    
    //pour qd d�passe les limites
    if(*x == 21)
        *x = 1;
    if(*x == 0)
        *x = 20;
    
    //regarde la tension du du joystick, si < ou > on posY ++ ou --
    if(tensionY > 200)
    {
        *y = *y - 1;
    }
    else if(tensionY < 50)
    {
        *y = *y + 1;
    }
    
    //pour qd d�passe les limites
    if(*y == 5)
        *y = 1;
    if(*y == 0)
        *y = 4;
            
    lcd_gotoXY(*x, *y);
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
    //si tombe sur une mine (game over))
    if(m_tabMines[y-1][x-1] == MINE)
        return false;
    
    //affiche sur tabVue la case de tabMines non min�e
    m_tabVue[y-1][x-1] = m_tabMines[y-1][x-1];
    
    if(m_tabMines[y-1][x-1] == ' ')
    {
        enleveTuilesAutour(x, y);
    }
    
    return true;
}
 
/*
 * @brief D�voile les cases non min�es autour de la tuile re�ue en param�tre.
 * Cette m�thode est appel�e par demine().
 * @param char x, char y Les positions X et y sur l'afficheur LCD.
 * @return rien
 */
void enleveTuilesAutour(char x, char y)
{
    //se r�f�rer � calculToucheCombien pour plus de d�tails
    signed char minY = -1;
    signed char maxY = 2;
    signed char minX = -1;
    signed char maxX = 2;
    
    if(y == 1)
        minY++;
    if(y == 4)
        maxY--;
    if(x == 1)
        minX++;
    if(x == 20)
        maxX--;
       
    for(signed char j = minX ; j < maxX ; j++)
    {
        for(signed char m = minY; m < maxY ; m++)
        {            
            if( m_tabMines[y+m-1][x+j-1] != MINE) //si pas min�e
            {
                m_tabVue[y+m-1][x+j-1] = m_tabMines[y+m-1][x+j-1]; //mets la case (vide)
            }
        }
    }    
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
    int nbCache = 0;
    
    for(int i = 0 ; i < 4 ; i++)
    {
        for(int k =0 ; k < 20 ; k++)
        {
            //regarde le nombre de tuiles encore pr�sentes
            if(m_tabVue[i][k] == TUILE)
                nbCache++;
        }
    }
    
    //si le nombre restant = le nombre de mines -> victoire
    if(nbCache == *pMines)
    {
        (*pMines)++;
        return true;
    }
    else
        return false;
}

/*
 * @brief Renvoie l'�tat actuel de la SW de la manette par true ou false
 * @param rien
 * @return bool
 */
bool testEtat(void)
{
    if(PORT_SW == 0) //si SW manette appuy�e retourne true
    {
        return true;
    }
    else
    {
        return false;
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
 
 
    /**************Timer 0*****************
    T0CONbits.TMR0ON    = 1;
    T0CONbits.T08BIT    = 0; // mode 16 bits
    T0CONbits.T0CS      = 0;
    T0CONbits.PSA       = 0; // prescaler enabled
    T0CONbits.T0PS      = 0b010; // 1:8 pre-scaler
    //TMR0 = DELAI_TMR0;
    INTCONbits.TMR0IE   = 1;  // timer 0 interrupt enable
    INTCONbits.TMR0IF   = 0; // timer 0 interrupt flag
    INTCONbits.PEIE = 1; //permet interruption des p�riph�riques
    INTCONbits.GIE = 1;  //interruptions globales permises
     * */
}
