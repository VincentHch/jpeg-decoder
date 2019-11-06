#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>



#include "jpeg_reader.h"
#include "huffman.h"
#include "bitstream.h"
#include "structure.h"
#include "decode_bloc.h"
#include "zig_zag.h"
#include "inverse_quantization.h"
#include "idct.h"
#include "conversion_couleurs.h"



void RGBtoYCbCr(uint8_t *tabYCbCr, uint8_t RGB[])
{
  /*
   *Entrée : tableau de 3 coefficients (Y, Cb, Cr)
   *Sortie : tableau de 3 coefficients (R, G, B)
   *R = Y + 1.402 × (C r − 128)
   *G = Y − 0.34414 × (C b − 128) − 0.71414 × (C r − 128)
   *B = Y + 1.772 × (C b − 128)
   */
    uint8_t Y = *(tabYCbCr+0);
    uint8_t Cb = *(tabYCbCr+1);
    uint8_t Cr = *(tabYCbCr+2);

    //ROUGE

    if (Y+1.402*(Cr-128)>255) {
      *(RGB+0) = 255;
    }
    else if (Y+1.402*(Cr-128)<0) {
      *(RGB+0) = 0;
    }
    else {
      *(RGB+0) = Y+1.402*(Cr-128);
    }

    //VERT
    
    if (Y-0.34414*(Cb-128)-0.71414*(Cr-128)>255) {
      *(RGB+1) = 255;
    }
    else if (Y-0.34414*(Cb-128)-0.71414*(Cr-128)<0) {
      *(RGB+1) = 0;
    }
    else {
      *(RGB+1) = Y-0.34414*(Cb-128)-0.71414*(Cr-128);
    }

    //BLEU
    
    if (Y+1.772*(Cb-128)>255) {
      *(RGB+2) = 255;
    }
    else if (Y+1.772*(Cb-128)<0) {
      *(RGB+2) = 0;
    }
    else {
      *(RGB+2) = Y+1.772*(Cb-128);
    }
}



struct RGB *conversion_mcu(struct MCU *mcu, int8_t nb_bloc)
{
    struct RGB *p_rgb = malloc(sizeof(struct RGB));
    uint8_t rgb_pixel[] = {0,0,0};
    uint8_t ycbcr_pixel[] = {0,0,0};

    for (int i_bloc=0; i_bloc<nb_bloc; i_bloc++){ //Pour chaque bloc (upsamplé) du mcu :
        for (int coord=0; coord<64; coord++){
            ycbcr_pixel[0] = (mcu->tabY)[i_bloc][coord];
            ycbcr_pixel[1] = (mcu->tabCb)[i_bloc][coord];  // On stocke le pixel "coord" du bloc "i_bloc"
            ycbcr_pixel[2] = (mcu->tabCr)[i_bloc][coord];

            RGBtoYCbCr(ycbcr_pixel, rgb_pixel);

            (p_rgb->tabR)[i_bloc][coord] = rgb_pixel[0];
            (p_rgb->tabG)[i_bloc][coord] = rgb_pixel[1];
            (p_rgb->tabB)[i_bloc][coord] = rgb_pixel[2];
        }
    }
    return p_rgb;
}


