#ifndef _STRUCTURE_H_
#define _STRUCTURE_H_



#include <stdio.h>
#include <stdint.h>



struct MCU{
  uint8_t tabY[4][64];
  uint8_t tabCb[4][64];
  uint8_t tabCr[4][64];
};



struct RGB{
  uint8_t tabR[4][64];
  uint8_t tabG[4][64];
  uint8_t tabB[4][64];
};



#endif
