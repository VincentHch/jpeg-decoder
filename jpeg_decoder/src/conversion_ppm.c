#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>



#include "jpeg_reader.h"
#include "huffman.h"
#include "bitstream.h"
#include "structure.h"
#include "decode_bloc.h"
#include "zig_zag.h"
#include "inverse_quantization.h"
#include "idct.h"
#include "conversion_couleurs.h"
#include "conversion_ppm.h"



/**********************************************************************************************************************************
 *                                                     PGM                                                                        *
 **********************************************************************************************************************************/



void completer_pixels_pgm(struct MCU *mcu, uint8_t id_bloc, uint8_t pixels[], int32_t x_top, int32_t y_left, int32_t largeur_image)
    /*
     *  On complète le tableau pixels[] en ajoutant les coordonnées du bloc données
     *  (nécessite les coordonnées du bloc)
     */
{
    for (int32_t x=0; x<8; x++)
    {
        for (int32_t y=0; y<8; y++)
        {
            //Pour le débogage : printf("x:%d, y=%d, indice=%d\n",x_top+x, y_left+y, (x_top + x)+largeur_image*(y+y_left));
            pixels[ (x_top+x) + largeur_image* (y+y_left) ] = (mcu->tabY)[id_bloc][x+8*y];
        }
    }
}



void remplir_pixels_pgm(struct MCU *liste_mcu[], struct jpeg_desc *jpeg, uint8_t pixels[])
    /*
     *  Tout est dit dans le nom
     */
{
    int32_t nb_mcu_par_ligne = get_nb_mcu(jpeg, DIR_H);
    int32_t nb_mcu_par_colonne = get_nb_mcu(jpeg, DIR_V);
    int32_t indice_mcu = 0;

    for (int32_t colonne = 0; colonne<nb_mcu_par_colonne; colonne++)
    {
        for (int32_t ligne = 0; ligne<nb_mcu_par_ligne; ligne++)
        {
            completer_pixels_pgm(liste_mcu[indice_mcu], 0,  pixels, ligne*8, colonne*8, nb_mcu_par_ligne*8);
            indice_mcu++;
        }
    }
}



void convert_pgm(struct MCU *liste_mcu[], struct jpeg_desc *jpeg, char *dest_filename)
{

    FILE* fichier = NULL;
    fichier = fopen(dest_filename, "w");

    if (fichier == NULL){
        fprintf(stderr, "Impossible de créer le fichier pgm.\n");
        exit(4);
    }
    else{

        //On récupère tous les attributs de l'image, et on alloue le tableau pixel
        int32_t nb_mcu_par_ligne = get_nb_mcu(jpeg, DIR_H);
        int32_t nb_mcu_par_colonne = get_nb_mcu(jpeg, DIR_V);
        int32_t nb_pixel = nb_mcu_par_ligne*nb_mcu_par_colonne*64;
        uint8_t *pixels;
        pixels = calloc(nb_pixel, sizeof(uint8_t));
        //Commande pas fonctionelle si la taille est trop grande : uint8_t pixels[nb_mcu_par_ligne*nb_mcu_par_colonne*64];

        remplir_pixels_pgm(liste_mcu, jpeg, pixels);

        int32_t y_max = get_image_size(jpeg, DIR_V);
        int32_t x_max = get_image_size(jpeg, DIR_H);

        fprintf(fichier,"P5\n%d %d\n255\n", x_max, y_max);

        /*Gestion de la magnifique barre de progression */
        int etoiles_a_afficher = 0;
        int etoiles_affichees = 0;
        int nb_espace = 50;
        printf("|");
        for (int i=0; i<nb_espace; i++)
        {
            printf(" ");
        }
        printf("|");
        printf("\033[%dD", nb_espace+1);

        uint8_t couleur;
        for (int32_t y=0; y<y_max; y++)
        {
            /*Gestion de la barre de progression*/
            etoiles_a_afficher = (y*(nb_espace+1))/y_max;
            while (etoiles_a_afficher > etoiles_affichees){
                printf("*\n");
                printf("\033[2A\n");
                printf("\033[%dC", etoiles_affichees+2);
                etoiles_affichees++;
            }
            for (int32_t x=0; x<x_max; x++)
            {
                couleur = pixels[x+y*(nb_mcu_par_ligne)*8];
                fwrite(&couleur, sizeof(uint8_t), 1, fichier);
                //Si on veut un ppm ascii, plus volumineux : fprintf(fichier, " %hhu\n", pixels[x+y*(nb_mcu_par_ligne)*8]);
                //Pour le débogage : printf("*** x:%d, y:%d, indice: %d\n", x, y, x+y*(nb_mcu_par_ligne)*8);
            }
        }
        free(pixels);
        fclose(fichier);

    }
}



/**********************************************************************************************************************************
 *                                                     PPM                                                                        *
 **********************************************************************************************************************************/



void completer_pixels_ppm(struct RGB *mcu, uint8_t id_bloc, uint8_t **pixels, int32_t x_top, int32_t y_left, int32_t largeur_image)
    /*
     *  On complète le tableau pixels[] en ajoutant les coordonnées du bloc données
     *  (nécessite les coordonnées du bloc)
     */
{
    for (int32_t x=0; x<8; x++)
    {
        for (int32_t y=0; y<8; y++)
        {
            //Pour le débogage : printf("x:%d, y=%d, id_bloc:%d, indice=%d\n",x_top+x, y_left+y, id_bloc, (x_top + x)+largeur_image*(y+y_left));
            pixels[ (x_top+x) + largeur_image* (y+y_left) ][0] = (mcu->tabR)[id_bloc][x+8*y];
            pixels[ (x_top+x) + largeur_image* (y+y_left) ][1] = (mcu->tabG)[id_bloc][x+8*y];
            pixels[ (x_top+x) + largeur_image* (y+y_left) ][2] = (mcu->tabB)[id_bloc][x+8*y];
        }
    }
}



void remplir_pixels_ppm(struct RGB *liste_mcu[], struct jpeg_desc *jpeg, uint8_t *pixels[])
{
    int32_t nb_mcu_par_ligne = get_nb_mcu(jpeg, DIR_H);
    int32_t nb_mcu_par_colonne = get_nb_mcu(jpeg, DIR_V);
    int32_t indice_mcu = 0;
    int8_t upsampl_H = get_frame_component_sampling_factor(jpeg, DIR_H, 0);
    int8_t upsampl_V = get_frame_component_sampling_factor(jpeg, DIR_V, 0);
    int32_t x_top;
    int32_t y_top;

    //On gère tous les cas d'echantillonage possible
    for (int32_t ligne = 0; ligne<nb_mcu_par_colonne; ligne++)
    {
        for (int32_t colonne = 0; colonne<nb_mcu_par_ligne; colonne++)
        {
            x_top = colonne*8*upsampl_H;
            y_top = ligne*8*upsampl_V;
            completer_pixels_ppm(liste_mcu[indice_mcu], 0,  pixels, x_top, y_top, nb_mcu_par_ligne*8*upsampl_H);
            if (upsampl_H == 2){
                completer_pixels_ppm(liste_mcu[indice_mcu], 1,  pixels, x_top+8, y_top, nb_mcu_par_ligne*8*upsampl_H);

                if (upsampl_V == 2){
                    completer_pixels_ppm(liste_mcu[indice_mcu], 2,  pixels, x_top, y_top+8, nb_mcu_par_ligne*8*upsampl_H);
                    completer_pixels_ppm(liste_mcu[indice_mcu], 3,  pixels, x_top+8, y_top+8, nb_mcu_par_ligne*8*upsampl_H);
                }
            }else if (upsampl_V == 2){
                completer_pixels_ppm(liste_mcu[indice_mcu], 1,  pixels, x_top, y_top+8, nb_mcu_par_ligne*8*upsampl_H);
            }
            indice_mcu++;
        }
    }
}

void convert_ppm(struct RGB *liste_mcu[], struct jpeg_desc *jpeg, char *dest_filename)
{

    FILE* fichier = NULL;
    fichier = fopen(dest_filename, "w");

    if (fichier == NULL){
        fprintf(stderr, "Impossible de créer le fichier pgm.\n");
        exit(4);
    }
    else{

        int32_t nb_mcu_par_ligne = get_nb_mcu(jpeg, DIR_H);
        int32_t nb_mcu_par_colonne = get_nb_mcu(jpeg, DIR_V);
        int8_t nb_bloc_par_mcu = get_nb_composantes(jpeg, 0);
        int32_t nb_pixel = nb_mcu_par_ligne*nb_mcu_par_colonne*64*nb_bloc_par_mcu;
        uint8_t **pixels;
        pixels = calloc(nb_pixel, sizeof(uint8_t *));
        for (int32_t x=0; x<nb_pixel; x++){
            pixels[x] = calloc(3, sizeof(uint8_t));
        }
        //La différence avec pgm est que pixels est un tableau de tableaux des 3 composantes rgb
        //uint8_t pixels[nb_mcu_par_ligne*nb_mcu_par_colonne*64][3];

        remplir_pixels_ppm(liste_mcu, jpeg, pixels);

        int32_t y_max = get_image_size(jpeg, DIR_V);
        int32_t x_max = get_image_size(jpeg, DIR_H);
        int8_t upsampl_H = get_frame_component_sampling_factor(jpeg, DIR_H, 0);

        fprintf(fichier,"P6\n%d %d\n255\n", x_max, y_max);

        uint8_t r,g,b;

        /*Gestion de la barre de progression*/
        int etoiles_a_afficher = 0;
        int etoiles_affichees = 0;
        int nb_espace = 50;
        printf("|");
        for (int i=0; i<nb_espace; i++)
        {
            printf(" ");
        }
        printf("|");
        printf("\033[%dD", nb_espace+1);

        for (int32_t y=0; y<y_max; y++)
        {
            /*Gestion de la barre de progression*/
            etoiles_a_afficher = (y*(nb_espace+1))/y_max;
            while (etoiles_a_afficher > etoiles_affichees){
                printf("*\n");
                printf("\033[2A\n");
                printf("\033[%dC", etoiles_affichees+2);
                etoiles_affichees++;
            }
            for (int32_t x=0; x<x_max; x++)
            {
                r = pixels[x+y*(nb_mcu_par_ligne)*8*upsampl_H][0];
                g = pixels[x+y*(nb_mcu_par_ligne)*8*upsampl_H][1];
                b = pixels[x+y*(nb_mcu_par_ligne)*8*upsampl_H][2];
                fwrite(&r, sizeof(uint8_t), 1, fichier);
                fwrite(&g, sizeof(uint8_t), 1, fichier);
                fwrite(&b, sizeof(uint8_t), 1, fichier);
                /*fprintf(fichier, " %hhu %hhu %hhu ", pixels[x+y*(nb_mcu_par_ligne)*8*upsampl_H][0],
                                                      pixels[x+y*(nb_mcu_par_ligne)*8*upsampl_H][1],
                                                      pixels[x+y*(nb_mcu_par_ligne)*8*upsampl_H][2]);
                */
                //printf("*** x:%d, y:%d, indice: %d\n", x, y, x+y*(nb_mcu_par_ligne)*8);
            }
        }

        //Libération de la mémoire allouée
        for (int32_t x=0; x<nb_pixel; x++){
            free(pixels[x]);
        }
        free(pixels);
        fclose(fichier);
    }
}
