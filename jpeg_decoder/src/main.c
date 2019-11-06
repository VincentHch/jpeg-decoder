#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>



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
#include "upsampling.h"
#include "parcours_bloc.h"
#include "main.h"



void remplacer_extension(const char *chaine,char *extension)
{
    char point = '.';
    int i=0;
    while (chaine[i]=='.')
    {
        i++;
    }
    char* p_point = strchr(chaine+i, point);
    if (p_point==NULL){
        fprintf(stderr, "Nom de fichier invalide\n");
        return;
    }

    for (int i=0; i<4; i++)
    {
        *(p_point + i + 1) = *(extension + i);
    }
}



int main(int argc, char **argv)
        /* Chargement du fichier jpeg */
{
    if (argc != 2) {
	    fprintf(stderr, "Usage: %s fichier.jpeg\n", argv[0]);
	    return EXIT_FAILURE;
    }

    char *filename = argv[1];
    struct jpeg_desc *jdesc = read_jpeg(filename);
    struct bitstream *stream = get_bitstream(jdesc);

    int8_t nb_Y = get_nb_composantes(jdesc, 0);
    int8_t nb_Cb = get_nb_composantes(jdesc, 1);
    //int8_t nb_Cr = get_nb_composantes(jdesc, 2);

    int16_t valeurs_prec[3] = {0,0,0};

    int32_t nb_total_mcu = get_nb_mcu(jdesc, DIR_H)*get_nb_mcu(jdesc, DIR_V);

    //printf("nb mcu : %d\n", nb_total_mcu);

    struct MCU *p_mcu[nb_total_mcu];
    struct RGB *p_rgb[nb_total_mcu];

    //Gestion de la barre de progression fabuleuse
    int etoiles_a_afficher = 0;
    int etoiles_affichees = 0;
    printf("Decodage en cours...\n");
    int nb_espace = 50;
    printf("|");
    for (int i=0; i<nb_espace; i++)
    {
        printf(" ");
    }
    printf("|");
    printf("\033[%dD", nb_espace+1);

    for (int32_t j = 0; j<nb_total_mcu; j++){    //On lit tous les blocs
      //printf("\n\n********* MCU %d ***********\n", j);
      p_mcu[j] = read_MCU(stream, jdesc, valeurs_prec);

      etoiles_a_afficher = (j*(nb_espace+1))/nb_total_mcu;
      while (etoiles_a_afficher > etoiles_affichees){
          printf("*\n");
          printf("\033[2A\n");
          printf("\033[%dC", etoiles_affichees+2);
          etoiles_affichees++;
      }



      if (nb_Cb>0){
          //printf("conversion bloc %d\n", j);
          p_rgb[j] = conversion_mcu(p_mcu[j], nb_Y);
      }

      /*
      Débogage (peut servir si on veut faire un jpeg2blabla) :

        for (int8_t i = 0; i<nb_Y; i++){
            //printf(" Composante Y n°%d :\n", i);
            //afficher_bloc_u8((p_mcu[j]->tabY)[i]);
            //printf("\n");
        }

        for (int8_t i = 0; i<nb_Cb; i++){
            //printf(" Composante Cb n°%d :\n", i);
            //afficher_bloc_u8((p_mcu[j]->tabCb)[i]);
            //printf("\n");
        }

        for (int8_t i = 0; i<nb_Cr; i++){
            //printf(" Composante Cr n°%d :\n", i);
            //afficher_bloc_u8((p_mcu[j]->tabCr)[i]);
            //printf("\n");
        }
        */
    }

    //char command[100];

    printf("\n");
    if (nb_Cb==0){
        printf("conversion pgm...\n");
        char *ext = "pgm";
        remplacer_extension(filename, ext);
        convert_pgm(p_mcu, jdesc, filename);
        //strcpy( command, "gnome-open ./resultat/image.pgm" );
        //system(command);
    }
    else{
        printf("conversion ppm...\n");
        char *ext = "ppm";
        remplacer_extension(filename, ext);
        convert_ppm(p_rgb, jdesc, filename);
        //strcpy( command, "gnome-open ./resultat/image.ppm" );
        //system(command);
    }

    /* Free des mcu*/
    for (int32_t j = 0; j<nb_total_mcu; j++){
      free(p_mcu[j]);
      if (nb_Cb>0){
          free(p_rgb[j]);
      }
    }


    printf("\n");
    close_jpeg(jdesc);
    return EXIT_SUCCESS;
}
