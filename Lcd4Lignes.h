/**
 * @file   Lcd4Lignes.h
 * @author Richard Cloutier
 * @date   Janvier 2014
 * @brief  Fichier de définition pour les différentes fonctions utilitaires pour le fonctionnement de l'afficheur LCD.
 * @version 1.0 : Adaptation du code fait par Christian Francoeur en décembre 2012 pour la carte BigPic
 */

#ifndef _LCD4LIGNES
#define _LCD4LIGNES

/**   
 *  Vous avez aussi les caratères suivants qui ne sont pas dans le standard ASCII
 *  À = 0         é = 1         É = 2        è = 3        È = 4    
 *  ê = 5         à = 6         â = 7
 * 
 *  Exemple: lcdPutMess("\1t\1");  ==> été
 */

#include <xc.h>
#include <stdbool.h>  // pour l'utilisation du type bool
#define _XTAL_FREQ 1000000 //Constante utilisée par __delay_ms(x). Doit = fréq interne du uC

/**
 * @brief Fonction d'initialisation. Elle doit être appelée avant de pouvoit utiliser les autres.
 */
void lcd_init(void);

/**
 * @brief Permet de modifier la position du curseur sur l'afficheur.
 * @param x valide entre 1 et 20
 * @param y valide entre 1 et 4
 */
void lcd_gotoXY(unsigned char x, unsigned char y);

/**
 * @brief Retourne le curseur à la position de base (en haut à gauche)
 */
void lcd_curseurHome(void);

/**
 * @brief Écriture du caractère spécifié à la position courante du curseur.
 * @param car Valeur ASCII que l'on veut afficher.
 */
void lcd_ecritChar(unsigned char car);

/**
 * @brief Permet l'affichage d'une chaîne de caractères à partir de la position courante du curseur.
 * @param chaine Chaîne qui contient les caractères ASCII à afficher. La valeur '\0' ne sera pas affichée.
 */
void lcd_putMessage(const unsigned char *chaine);

/**
 * @brief Efface l'affichage et retourne le curseur à la position intiale.
 */
void lcd_effaceAffichage(void);                     

/**
 * @brief Efface une ligne complète sur l'afficheur.
 * @param y Numéro de la ligne à effacer. Valide entre 1 et 4.
 */
void lcd_effaceLigne(unsigned char y);              

/**
 * @brief Effaçage à partir du curseur.
 * @param nbr Nombre de caractères à effacer.
 */
void lcd_effaceChar(unsigned char nbr);             // Effacer nombre de caractères

/**
 * @brief Cacher le curseur.
 */
void lcd_cacheCurseur(void);

/**
 * @brief Afficher le curseur.
 */
void lcd_montreCurseur(void);

#endif