#ifndef ___PARCOURS_H__
#define ___PARCOURS_H__

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
/*
#include "jpeg_reader.h"
#include "huffman.h"
#include "inverse_quantization.h"
#include "bitstream.h"
#include "decode_bloc.h"
#include "zig_zag.h"
#include "idct.h"
//#include "conversion_ppm.h"
#include "upsampling.h"
*/



/*
struct MCU{
  uint8_t tabY[4][64];
  uint8_t tabCb[4][64];
  uint8_t tabCr[4][64];
};
*/

int32_t get_nb_mcu(struct jpeg_desc *jpeg, enum direction dir);

void copier_bloc(uint8_t *bloc, uint8_t *bloc_a_copier);

int8_t get_nb_composantes(struct jpeg_desc *jpeg, int8_t i);

struct MCU *first_read_MCU(struct bitstream *stream, struct jpeg_desc *jpeg, int16_t *valeurs_prec);

void read_MCU(struct bitstream *stream, struct jpeg_desc *jpeg, struct MCU *p_mcu);
#endif
