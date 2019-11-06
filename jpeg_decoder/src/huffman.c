#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "bitstream.h"



struct huff_table {

    int8_t valeur;
    bool est_occupe;
    struct huff_table *fils_droit;
    struct huff_table *fils_gauche;
};



struct huff_table *nouveau_noeud(uint8_t valeur, bool est_occupe)
    /* On crée un nouveau noeud avec les paramètres donnés */
{ 
    struct huff_table *noeud = malloc(sizeof(struct huff_table));
    noeud->est_occupe = est_occupe;
    noeud->valeur = valeur;
    noeud->fils_droit = NULL;
    noeud->fils_gauche = NULL;

    return noeud;
}

bool ajouter_noeud_gauche( uint8_t valeur, uint8_t profondeur, struct huff_table *racine)
    /* Dans cette fonction, on ajoute un noeud le plus à gauche possible
     * sans bouleverser la structure de l'arbre de Huffman */
{
    if (racine->est_occupe == true) { //si noeud déjà occupé on ne peut pas insérer
        return false;
    }

    if (profondeur == 1) {

        if (racine->fils_gauche == NULL) {
            racine->fils_gauche = nouveau_noeud(valeur, true);  //on ajoute à gauche
            return true;
        }

        else if (racine->fils_droit == NULL) {
            racine->fils_droit = nouveau_noeud(valeur, true); //si déjà occupé à gauche on remplit à droite
            return true;
        }

        else {
            return false;
        }
    }

    if (racine->fils_gauche == NULL) { //si on est pas encore à la bonne profondeur et qu'il y a de la place à gauche
        racine->fils_gauche = nouveau_noeud(0, false);
        bool est_insere = ajouter_noeud_gauche(valeur, profondeur - 1, racine->fils_gauche); //on vérifie que l'on a bien pu insérer l'élément

        if (est_insere == true) {
            return true; //on a fini l'insertion
        }
    }

    else { //si à gauche ce n'est pas vide on continue quand même
        bool est_insere = ajouter_noeud_gauche(valeur, profondeur - 1, racine->fils_gauche);

        if (est_insere == true) {
            return true; //on a fini l'insertion
        }

    }

    if (racine->fils_droit == NULL) { //si on est pas arrivé à insérer à gauche on bifurque à droite
        racine->fils_droit = nouveau_noeud(0, false);
        bool est_insere = ajouter_noeud_gauche(valeur, profondeur - 1, racine->fils_droit);

        if (est_insere == true) {
            return true;
        }
    }

    else { //si à droite ce n'est pas vide, on continue quand même
        bool est_insere = ajouter_noeud_gauche(valeur, profondeur - 1, racine->fils_droit);

    return est_insere; //on aura fini toute les possibilités d'insertion
    }

    fprintf(stderr, "Erreur insetion noeud Huffman");
    exit(2);
}


    
struct huff_table *load_huffman_table(struct bitstream *stream, uint16_t *nb_byte_read)
    /* On charge la table de Huffman désirée dans notre structure sous
     * forme d'arbre binaire, ce qui est plus simple pour la lecture, on
     * oublie pas que seule les magnitudes et les simboles RLE sont codés
     * donc on a besoin de seulement 1 octet au maximum */
{
    //on lit d'abord les profondeurs
    uint32_t profondeurs[16];

    for (int k = 0; k < 16; k++) {
        uint8_t nb_lu = 0;
        nb_lu = read_bitstream(stream, 8, &(profondeurs[k]), false);
        *nb_byte_read += 1; //on incrémente

        if (nb_lu != 8) {
            fprintf(stderr, "Erreur lecture bitstream Huffman\n");
            exit(2);
        }
    }
    //enfin on lit les valeurs
    uint32_t **tableau_valeur; //tableau_valeur[i][j] --> jieme mot codé en profondeur i
    tableau_valeur = calloc(sizeof(uint32_t *), 16);

    for (int k = 0; k < 16; k++) {

        tableau_valeur[k] = calloc(sizeof(uint32_t), profondeurs[k]); //on alloue le bon nombre d'espace pour le tableau

        for (int i = 0; i < (int) profondeurs[k]; i++) {

            uint8_t nb_lu = 0;
            nb_lu = read_bitstream(stream, 8, &(tableau_valeur[k][i]), false);
            *nb_byte_read += 1;
            if (nb_lu != 8) {
                fprintf(stderr, "Erreur lecture bitstream Huffman\n");
                exit(2);
            }
        }
    }

    /* Maintenant il faut remplir l'arbre à l'aide de la table */

    struct huff_table *racine = malloc(sizeof(struct huff_table)); //on initialise la racine

    racine->valeur = 0;
    racine->est_occupe = false;
    racine->fils_droit = NULL;
    racine->fils_gauche = NULL;

    for (int k = 0; k < 16; k++) {
        for (int i = 0; i < (int) profondeurs[k]; i++) {
            bool insertion =  ajouter_noeud_gauche(tableau_valeur[k][i], k+1, racine); //on insère le noeud le plus à gauche possible

            if (insertion == false) {
                fprintf(stderr, "Erreur insertion arbre Huffman\n");
                exit(2);
            }
        } 

        free(tableau_valeur[k]); //on libère la mémoire allouée au fur et à mesure
    }
    free(tableau_valeur);
    return racine;
}

int8_t next_huffman_value(struct huff_table *table, struct bitstream *stream)
    /* Il suffit de parcourir l'arbre à gauche ou à droite
     * en fonction des bit du bitstream est s'arrêter lorsque l'on tombre
     * sur une feuille */
{
    uint8_t nb_lu; //nombre de bit lu pour le code
    struct huff_table *noeud_en_cours = table;
    while (noeud_en_cours->est_occupe == false) {
        uint32_t valeur_lue;
        nb_lu = read_bitstream(stream, 1, &valeur_lue, true);

        if (nb_lu != 1) {
            fprintf(stderr, "erreur lecture bitstream huffman value");
            exit(2);
        }

        if (valeur_lue == 0) { //il faut se déplacer à gauche
            noeud_en_cours = noeud_en_cours->fils_gauche;
        }

        else { //dans ce cas on se déplace à droite
            noeud_en_cours = noeud_en_cours->fils_droit;
        }
    }

       return noeud_en_cours->valeur;
}


void free_huffman_table(struct huff_table *racine)
    /* On parcourt l'arbre en libérant l'espace
     * mémoire */
{
    if (racine != NULL) {
        free_huffman_table(racine->fils_droit); //récursion pour libérer les fils
        free_huffman_table(racine->fils_gauche);
        free(racine); //on libère la racine
    }

}



