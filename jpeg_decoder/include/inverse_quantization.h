#ifndef __INVERSEQUANT_H__
#define __INVERSEQUANT_H__



#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "jpeg_reader.h"
#include "bitstream.h"
#include "parcours_bloc.h"



void i_quantization(struct jpeg_desc *jpeg, int16_t *bloc, uint8_t indice);



#endif
