#ifndef __IDCT_H__
#define __IDCT_H__



#include <stdlib.h>
#include <stdio.h>
#include <math.h>



#include "jpeg_reader.h"
#include "bitstream.h"
#include "decode_bloc.h"
#include "zig_zag.h"



uint8_t norme(float x);

float coefficient(int x);

uint8_t idct_x_y(int x,int y,int16_t *bloc_freq );

void IDCT(int16_t *bloc_freq, uint8_t *bloq_idct); 



#endif
