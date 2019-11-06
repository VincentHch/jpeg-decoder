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



int test_quantification(int argc, char **argv) //à changer en main si l'on veut tester la quantification
{
    if (argc != 2) {

    fprintf(stderr, "Usage: %s fichier.jpeg\n", argv[0]);
	return 0;
    }

    const char *filename = argv[1];

    struct jpeg_desc *jdesc = read_jpeg(filename);

    struct bitstream *stream = get_bitstream(jdesc);

    int16_t bloc[64];
    read_bloc(stream, jdesc, bloc, 0 ,0);
    int16_t valeur_prec = bloc[0];
    i_quantization(jdesc, bloc, 0);
    for (int i=0; i<64; i++){
      printf("%hx, ", bloc[i]);
    }
    printf("\n");


    printf("%hx\n***\n", valeur_prec);
    read_bloc(stream, jdesc, bloc, 0 , valeur_prec);
    for (int i=0; i<64; i++){
        printf("%hx, ", bloc[i]);
    }
    printf("\n");
    i_quantization(jdesc, bloc, 0);
    for (int i=0; i<64; i++){
      printf("%hx, ", bloc[i]);
    }
    printf("\n");

    close_jpeg(jdesc);
    return 0;
}
