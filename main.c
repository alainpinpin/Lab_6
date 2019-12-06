 /**
 * @file   main.c  Lab_6_AA_Demineur
 * @author Alexandre Alain
 * @date   6 Décembre 2019
 * @brief  Jeux de Démineur refait à la TSO. Le numéro du laboratoire et le nom du programmeur 
  * s'affiche à l'écran sur un cours laps de temps et ensuite le jeu commence. L'écran est remplie 
  * de tuiles. La position du joueur <<flash>>, il se déplace grâce au <<joystick>>. Si le joueur 
  * appuie sur l'interrupteur du <<joystick>> cela démine la case et les vides autour.
  * Si la case était une mine alors la partie est perdu. L'écran affiche alors les mines et les chiffres.
  * Le joueur peut recomencer une nouvelle partie en appuyant de nouveau sur la manette.
  * Au début il y a 5 mines de cachées, le nombre augmente de un à chaque partie gagnée.
  * La partie est gagnée si toutes les cases, sauf les mines, sont déminées. Les mines 
  * ils apparaissent et on appuie pour une nouvelle partie.
 *
 * @version 1.0
 * Environnement:
 *     Développement: MPLAB X IDE (version 5.05)
 *     Compilateur: XC8 (version 2.10)
 *     Matériel: Carte démo du Pickit3. PIC 18F45K20
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
#define _XTAL_FREQ 1000000 //Constante utilisée par __delay_ms(x). Doit = fréq interne du uC
#define NB_LIGNE 4  //afficheur LCD 4x20
#define NB_COL 20
#define AXE_X 7  //canal analogique de l'axe x
#define AXE_Y 6
#define PORT_SW PORTBbits.RB1 //sw de la manette
#define TUILE 1 //caractère cgram d'une tuile
#define MINE 2 //caractère cgram d'une mine
#define DRAPEAU 3 //caractère cgram d'un drapeau
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
char m_tabMines[NB_LIGNE][NB_COL+1]; //Tableau des caractères affichés au LCD
char m_tabVue[NB_LIGNE][NB_COL+1]; //Tableau contenant les mines, les espaces et les chiffres

/*               ***** PROGRAMME PRINCPAL *****                             */
void main(void)
{
    /*********variables locales du main********/
    const char afficheNom[] = "Lab6 Alexandre Alain";  
    int posX = 10; //posisition du joueur, initialement à 10/3
    int posY = 3;     
    bool etatInitSW = false; //servira à éviter que le joueur laisse le bouton appuyé
    bool etatAfterSW = false;
    int nbMines = STARTINGBOMBS;
    
    /*********début du code********************/
    initialisation(); //initialise les entrés/sorties
    lcd_init();    
       
    etatInitSW = testEtat(); //mets l'état de la swicth avant/après à jour
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
        
        deplace(&posX, &posY); //déplace le joueur (duh...)
                
        etatInitSW = testEtat(); //regarde l'état de la switch
        
        if( (etatInitSW != etatAfterSW) && (etatAfterSW == false) ) //si SW manette apuyé, trouvé c'est lequel
        {
                   
            if( (demine(posX, posY) == false) || ( gagne(&nbMines) == true) )
            {
                lcd_cacheCurseur(); //par soucie d'esthétique
                afficheTabMines(); //montre tableau avec mines et chiffres
                __delay_ms(500);//délai pour eviter de restarter trop vite
                
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
        
        etatAfterSW = etatInitSW; //enregistre le dernier état
        
        /*Bonus du drapeau : fait apparaitre un drapeau flottant au vent si on appuie sur la SW1
        de la carte alors que le joueur se trouve sur une tuile. Remets une tuile si 
        il y a déjà un drapeau. */
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
            
            while (PORTBbits.RB0 == 0);//on attend que le bouton soit relâché
            
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
 * @brief Rempli le tableau m_tabVue avec le caractère spécial (définie en CGRAM
 *  du LCD) TUILE. Met un '\0' à la fin de chaque ligne pour faciliter affichage
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
 *  mine contiendront le caractère MINE défini en CGRAM.
 * @param int nb, le nombre de mines à mettre dans le tableau 
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
          
    //mets les mines dans la matrice m_tabMines aléatoirement
    for(int i = 0 ; i < nb ; i++)
    {
        do
        {            
            testX = rand()%20;
            testY = rand()%4;
        }
        while(tabSafe[testY][testX] == true);// s'assure que le case n'est pas déjà occupée

        tabSafe[testY][testX] = true;//mets la case à true pour éviter de réécrire par dessus
        m_tabMines[testY][testX] = MINE; //mets une mine à l'endroit vide
    }  
}
 
/*
 * @brief Rempli le tableau m_tabMines avec le nombre de mines que touche la case.
 * Si une case touche à 3 mines, alors la méthode place le code ascii de 3 dans
 * le tableau. Si la case ne touche à aucune mine, la méthode met le code
 * ascii d'un espace.
 * Cette méthode utilise calculToucheCombien(). 
 * @param rien
 * @return rien
 */
void metToucheCombien(void)
{
    char chiffre = 0; // chiffre à afficher
    
    for(int j = 0 ; j < NB_COL ; j++)
    {
        for(int m = 0; m < NB_LIGNE ; m++)
        {
            chiffre = calculToucheCombien(m, j);
                        
            if( chiffre != 0 && m_tabMines[m][j] != MINE) //si case différente d'une mine, met le chiffre
            {
                m_tabMines[m][j] = 48 + chiffre;
            }
             
        }
    }    
}
 
/*
 * @brief Calcul à combien de mines touche la case.
 * @param int ligne, int colonne La position dans le tableau m_tabMines a vérifier
 * @return char nombre. Le nombre de mines touchées par la case
 */
char calculToucheCombien(int ligne, int colonne)
{
    unsigned char nombre = 0;
    signed char minLigne = -1;
    signed char maxLigne = 2;
    signed char minCol = -1;
    signed char maxCol = 2;
    
    //gère les différents cas limites sur les bords
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
                nombre++; //incrémente le nombre si une mine est autour
            }
        }
    }    
    
    return nombre;
}
 
/**
 * @brief Si la manette est vers la droite ou la gauche, on déplace le curseur 
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
    
    //pour qd dépasse les limites
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
    
    //pour qd dépasse les limites
    if(*y == 5)
        *y = 1;
    if(*y == 0)
        *y = 4;
            
    lcd_gotoXY(*x, *y);
}
 
/*
 * @brief Dévoile une tuile (case) de m_tabVue. 
 * S'il y a une mine, retourne Faux. Sinon remplace la case et les cases autour
 * par ce qu'il y a derrière les tuiles (m_tabMines).
 * Utilise enleveTuileAutour().
 * @param char x, char y Les positions X et y sur l'afficheur LCD
 * @return faux s'il y avait une mine, vrai sinon
 */
bool demine(char x, char y)
{
    //si tombe sur une mine (game over))
    if(m_tabMines[y-1][x-1] == MINE)
        return false;
    
    //affiche sur tabVue la case de tabMines non minée
    m_tabVue[y-1][x-1] = m_tabMines[y-1][x-1];
    
    if(m_tabMines[y-1][x-1] == ' ')
    {
        enleveTuilesAutour(x, y);
    }
    
    return true;
}
 
/*
 * @brief Dévoile les cases non minées autour de la tuile reçue en paramètre.
 * Cette méthode est appelée par demine().
 * @param char x, char y Les positions X et y sur l'afficheur LCD.
 * @return rien
 */
void enleveTuilesAutour(char x, char y)
{
    //se référer à calculToucheCombien pour plus de détails
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
            if( m_tabMines[y+m-1][x+j-1] != MINE) //si pas minée
            {
                m_tabVue[y+m-1][x+j-1] = m_tabMines[y+m-1][x+j-1]; //mets la case (vide)
            }
        }
    }    
}
 
/*
 * @brief Vérifie si gagné. On a gagné quand le nombre de tuiles non dévoilées
 * est égal au nombre de mines. On augmente de 1 le nombre de mines si on a 
 * gagné.
 * @param int* pMines. Le nombre de mine.
 * @return vrai si gagné, faux sinon
 */
bool gagne(int* pMines)
{
    int nbCache = 0;
    
    for(int i = 0 ; i < 4 ; i++)
    {
        for(int k =0 ; k < 20 ; k++)
        {
            //regarde le nombre de tuiles encore présentes
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
 * @brief Renvoie l'état actuel de la SW de la manette par true ou false
 * @param rien
 * @return bool
 */
bool testEtat(void)
{
    if(PORT_SW == 0) //si SW manette appuyée retourne true
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
 * @param Le no du port à lire
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
 * @brief Fait l'initialisation des différents regesitres et variables.
 * @param Aucun
 * @return Aucun
 */
void initialisation(void)
{
    TRISD = 0; //Tout le port D en sortie
 
    ANSELH = 0;  // RB0 à RB4 en mode digital. Sur 18F45K20 AN et PortB sont sur les memes broches
    TRISB = 0xFF; //tout le port B en entree
 
    ANSEL = 0;  // PORTA en mode digital. Sur 18F45K20 AN et PortA sont sur les memes broches
    TRISA = 0; //tout le port A en sortie
 
    //Pour du vrai hasard, on doit rajouter ces lignes. 
    //Ne fonctionne pas en mode simulateur.
    T1CONbits.TMR1ON = 1;
    srand(TMR1);
 
   //Configuration du port analogique
    ANSELbits.ANS7 = 1;  //A7 en mode analogique
 
    ADCON0bits.ADON = 1; //Convertisseur AN à on
	ADCON1 = 0; //Vref+ = VDD et Vref- = VSS
 
    ADCON2bits.ADFM = 0; //Alignement à gauche des 10bits de la conversion (8 MSB dans ADRESH, 2 LSB à gauche dans ADRESL)
    ADCON2bits.ACQT = 0;//7; //20 TAD (on laisse le max de temps au Chold du convertisseur AN pour se charger)
    ADCON2bits.ADCS = 0;//6; //Fosc/64 (Fréquence pour la conversion la plus longue possible)
 
 
    /**************Timer 0*****************
    T0CONbits.TMR0ON    = 1;
    T0CONbits.T08BIT    = 0; // mode 16 bits
    T0CONbits.T0CS      = 0;
    T0CONbits.PSA       = 0; // prescaler enabled
    T0CONbits.T0PS      = 0b010; // 1:8 pre-scaler
    //TMR0 = DELAI_TMR0;
    INTCONbits.TMR0IE   = 1;  // timer 0 interrupt enable
    INTCONbits.TMR0IF   = 0; // timer 0 interrupt flag
    INTCONbits.PEIE = 1; //permet interruption des périphériques
    INTCONbits.GIE = 1;  //interruptions globales permises
     * */
}
