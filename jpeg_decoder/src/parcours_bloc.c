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
#include "upsampling.h"
#include "parcours_bloc.h"



void copier_bloc(uint8_t *bloc, uint8_t *bloc_a_copier)
{
    for (int i=0; i<64; i++){
    bloc[i] = bloc_a_copier[i];
  }
}



int32_t get_nb_mcu(struct jpeg_desc *jpeg, enum direction dir)
  /*Renvoie le nombre de MCU dans l'image selon la direction dir
   *nb = ceil(size(dir)/(8.0*sample_factor(0,dir))
   */
{
    float size = get_image_size(jpeg, dir);
    float nb_pixel_mcu = 8.0*get_frame_component_sampling_factor(jpeg, dir, 0);
    int32_t nb = ceil(size/nb_pixel_mcu);
    return nb;
}


int8_t get_nb_composantes(struct jpeg_desc *jpeg, int8_t i)
  /*
  renvoie le nombre de bloc de composante i par mcu
  */
{
    return get_frame_component_sampling_factor(jpeg, DIR_V, i)
            *get_frame_component_sampling_factor(jpeg, DIR_H, i);
}



struct MCU *read_MCU(struct bitstream *stream, struct jpeg_desc *jpeg, int16_t *valeurs_prec)
    /*
     * On lit tous les blocs du prochain MCU, stockée dans une structure MCU
     * D'abord, tous les Y
     * Puis, si ils existent, tous les Cb et Cr
     */

{
    //Creation du mcu, ne pas oublier de free plus tard !
    struct MCU *p_mcu;
    p_mcu = malloc(sizeof(struct MCU));
    uint8_t indice_couleur;
    //On lit les Y
    indice_couleur = 0;
    int8_t nb_Y = get_nb_composantes(jpeg, indice_couleur);


    //Penser a appliquer les corrections aussi pour CB et Cr !!!

    int16_t valeur_prec_Y;
    int16_t valeur_prec_Cb;
    int16_t valeur_prec_Cr;
    //Lecture des valeurs précédentes
    valeur_prec_Y = valeurs_prec[0];
    valeur_prec_Cb = valeurs_prec[1];
    valeur_prec_Cr = valeurs_prec[2];



  //printf("on lit Y\n");
    for (int8_t i = 0; i<nb_Y; i++){

        int16_t bloc[64]; //type a revoir
        read_bloc(stream, jpeg, bloc, indice_couleur, valeur_prec_Y);
        valeur_prec_Y = bloc[0]; //On met à jour la valeur précédente de DC

        i_quantization(jpeg, bloc, indice_couleur); //Quantization inverse

        int16_t bloc_izz[64];
        d_zig_zag(bloc, bloc_izz); //Zig-zag inverse

        uint8_t bloc_idct[64];;
        IDCT(bloc_izz, bloc_idct);  //IDCT

        copier_bloc((p_mcu->tabY)[i], bloc_idct);; //tabY[i] = bloc
    }

  //Si l'image est en couleur, on lit Cb et Cr
    if (get_nb_components(jpeg)>2){


        indice_couleur = 1;

        int8_t nb_Cb = get_nb_composantes(jpeg, indice_couleur);
        //On lie les Cb
        for (int8_t i = 0; i<nb_Cb; i++){

            int16_t bloc[64]; //type a revoir
            read_bloc(stream, jpeg, bloc, indice_couleur, valeur_prec_Cb);
            valeur_prec_Cb = bloc[0];

            i_quantization(jpeg, bloc, indice_couleur);//Quantization inverse

            int16_t bloc_izz[64];
            d_zig_zag(bloc, bloc_izz); //Zig-zag inverse

            uint8_t bloc_idct[64];
            IDCT(bloc_izz, bloc_idct); //IDCT

            copier_bloc((p_mcu->tabCb)[i], bloc_idct); //tabY[i] = bloc
        }

    //lecture des Cr
        indice_couleur = 2;
        int8_t nb_Cr = get_nb_composantes(jpeg, indice_couleur);

        for (int8_t i = 0; i<nb_Cr; i++){
            int16_t bloc[64]; //type a revoir
            read_bloc(stream, jpeg, bloc, indice_couleur, valeur_prec_Cr);
            valeur_prec_Cr = bloc[0];

            i_quantization(jpeg, bloc, indice_couleur);//Quantization inverse

            int16_t bloc_izz[64]; //Zig-zag inverse
            d_zig_zag(bloc, bloc_izz);

            uint8_t bloc_idct[64];
            IDCT(bloc_izz, bloc_idct); //IDCT
            copier_bloc((p_mcu->tabCr)[i], bloc_idct); //tabY[i] = bloc


        }
    }

    upsampling_MCU(p_mcu, jpeg);//upsampling

    valeurs_prec[0] = valeur_prec_Y;
    valeurs_prec[1] = valeur_prec_Cb;
    valeurs_prec[2] = valeur_prec_Cr;
    return p_mcu;
}



void afficher_bloc(int16_t *bloc)
{
    for (int i=0; i<64; i++){

        printf("%hx, ", bloc[i]);
    }
    printf("\n");
}



void afficher_bloc_u8(uint8_t *bloc)
{
    for (int i=0; i<64; i++){

        printf("%hhx, ", bloc[i]);
    }
    printf("\n");
}
