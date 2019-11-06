#ifndef ___UPSAMPLING_H__
#define ___UPSAMPLING_H__



#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>



#include "jpeg_reader.h"
#include "parcours_bloc.h"
#include "conversion_ppm.h"



void etire_droite(uint8_t *composante_a_etirer, uint8_t *composante_1, uint8_t *composante_2);



void etire_bas(uint8_t *composante_a_etirer, uint8_t *composante_1, uint8_t *composante_2);



void upsampling_MCU(struct MCU *mcu, struct jpeg_desc *jpeg);



#endif
