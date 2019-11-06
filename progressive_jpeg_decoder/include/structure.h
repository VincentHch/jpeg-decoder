#ifndef _STRUCTURE_H_
#define _STRUCTURE_H_


#include <stdio.h>
#include <stdint.h>

struct MCU{
  uint8_t tabY[4][64];
  uint8_t tabCb[4][64];
  uint8_t tabCr[4][64];
  uint8_t idctY[4][64];
  uint8_t idctCb[4][64];
  uint8_t idctCr[4][64];
  int16_t brutY[4][64];
  int16_t brutCb[4][64];
  int16_t brutCr[4][64];
};

struct RGB{
  uint8_t tabR[4][64];
  uint8_t tabG[4][64];
  uint8_t tabB[4][64];
};

#endif
