#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "bitstream.h"
#include "decode_bloc.h"
#include "huffman.h"
#include "jpeg_reader.h"
#include "structure.h"
#include "zig_zag.h"

/* On fait le zig_zag inverse
 * en gardant un tableau à une dimension
 * on ne fournie qu'une fonction qui permet
 * de garder la structure 1D mais ils faut alors
 * changer les indices.
 * */

void d_zig_zag(int16_t bloc[], int16_t new_bloc[]) {

  int tab_indice[8][8] = {
      {0, 1, 5, 6, 14, 15, 27, 28},     {2, 4, 7, 13, 16, 26, 29, 42},
      {3, 8, 12, 17, 25, 30, 41, 43},   {9, 11, 18, 24, 31, 40, 44, 53},
      {10, 19, 23, 32, 39, 45, 52, 54}, {20, 22, 33, 38, 46, 51, 55, 60},
      {21, 34, 37, 47, 50, 56, 59, 61}, {35, 36, 48, 49, 57, 58, 62, 63}};

  int k = 0;
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {

      new_bloc[k] = bloc[tab_indice[i][j]];
      k++;
    }
  }
}

int tab_indices_i[64][2] = {
    {0, 0}, {0, 1}, {1, 0}, {2, 0}, {1, 1}, {0, 2}, {0, 3}, {1, 2},
    {2, 1}, {3, 0}, {4, 0}, {3, 1}, {2, 2}, {1, 3}, {0, 4}, {0, 5},
    {1, 4}, {2, 3}, {3, 2}, {4, 1}, {5, 0}, {6, 0}, {5, 1}, {4, 2},
    {3, 3}, {2, 4}, {1, 5}, {0, 6}, {0, 7}, {1, 6}, {2, 5}, {3, 4},
    {4, 3}, {5, 2}, {6, 1}, {7, 0}, {7, 1}, {6, 2}, {5, 3}, {4, 4},
    {3, 5}, {2, 6}, {1, 7}, {2, 7}, {3, 6}, {4, 5}, {5, 4}, {6, 3},
    {7, 2}, {7, 3}, {6, 4}, {5, 5}, {4, 6}, {3, 7}, {4, 7}, {5, 6},
    {6, 5}, {7, 4}, {7, 5}, {6, 6}, {5, 7}, {6, 7}, {7, 6}, {7, 7}};



void store_dezigzag_coordonnes(int coordonne, int *x, int *y) {
    *x = tab_indices_i[coordonne][0];
    *y = tab_indices_i[coordonne][1];
}
