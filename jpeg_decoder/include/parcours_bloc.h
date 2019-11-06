#ifndef ___PARCOURS_H__
#define ___PARCOURS_H__



#include <stdlib.h>
#include <stdio.h>
#include <math.h>



int32_t get_nb_mcu(struct jpeg_desc *jpeg, enum direction dir);

void copier_bloc(uint8_t *bloc, uint8_t *bloc_a_copier);

int8_t get_nb_composantes(struct jpeg_desc *jpeg, int8_t i);

struct MCU *read_MCU(struct bitstream *stream, struct jpeg_desc *jpeg, int16_t *valeurs_prec);



#endif
