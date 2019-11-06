#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "jpeg_reader.h"
#include "huffman.h"
#include "bitstream.h"
#include "structure.h"
#include "decode_bloc.h"
#include "zig_zag.h"
#include "inverse_quantization.h"



void i_quantization(struct jpeg_desc *jpeg, int16_t *bloc, uint8_t indice) 
{
    uint8_t indice_table = get_frame_component_quant_index (jpeg , indice); //On récupère l'indice de la table
    uint8_t *quantization_table = get_quantization_table(jpeg, indice_table);

    for (uint32_t k = 0; k < 64; k++) {

        uint8_t mult_q = quantization_table[k]; //Opération de inverse de la quantization
        bloc[k] *= mult_q;

    }
}



