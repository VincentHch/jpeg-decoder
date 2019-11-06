#ifndef __DECODEBLOC_H__
#define __DECODEBLOC_H__



#include <stdint.h>
#include <stdbool.h>



void read_bloc(struct bitstream *stream, struct jpeg_desc *jpeg, int16_t bloc[], uint8_t indice_couleur, int16_t valeur_prec);

int16_t amplitude(int8_t m, uint32_t indice);

void read_AC(struct bitstream *stream, struct jpeg_desc *jpeg, int16_t bloc[], uint8_t indice_couleur);

int16_t read_DC(struct bitstream *stream, struct jpeg_desc *jpeg, uint8_t indice_couleur, int16_t valeur_prec);



#endif
