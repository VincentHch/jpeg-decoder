#ifndef _PPM_H_
#define _PPM_H_



#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>



#include "jpeg_reader.h"
#include "huffman.h"
#include "bitstream.h"
#include "structure.h"



void convert_pgm(struct MCU *liste_mcu[], struct jpeg_desc *jpeg, char *dest_filename);
void convert_ppm(struct RGB *liste_mcu[], struct jpeg_desc *jpeg, char *dest_filename);



#endif
