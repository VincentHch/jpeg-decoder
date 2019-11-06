#ifndef __ZIGZAG_H__
#define __ZIGZAG_H__

#include "parcours_bloc.h"

int tab_indices_i[64][2];
void d_zig_zag(int16_t bloc[], int16_t new_bloc[]);
void store_dezigzag_coordonnes(int coordonne, int *x, int *y);
#endif
