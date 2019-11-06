#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>



#include "jpeg_reader.h"
#include "huffman.h"
#include "bitstream.h"
#include "structure.h"
#include "decode_bloc.h"
#include "zig_zag.h"



/* On fait le zig_zag inverse
 * en gardant un tableau à une dimension
 * on ne fournie qu'une fonction qui permet
 * de garder la structure 1D mais ils faut alors
 * changer les indices.
 * */



void d_zig_zag(int16_t bloc[], int16_t new_bloc[])
{

    int tab_indice[8][8]= {{0, 1, 5, 6, 14, 15, 27, 28 },
                        {2, 4, 7, 13, 16, 26, 29, 42},
                        {3, 8, 12, 17, 25, 30, 41, 43},
                        {9, 11, 18, 24, 31, 40, 44, 53},
                        {10, 19, 23, 32, 39, 45, 52, 54},
                        {20, 22, 33, 38, 46, 51, 55, 60},
                        {21, 34, 37, 47, 50, 56, 59, 61},
                        {35, 36, 48, 49, 57, 58, 62, 63}};

    int k = 0;

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {

            new_bloc[k] = bloc[tab_indice[i][j]];
            k++;
        }
    }
}



int test_zig_zag(int argc, char **argv) //a changer en main si l'on veut tester zig_zag
    /*Test de zig_zag*/
{
    if (argc != 2) {
	fprintf(stderr, "Usage: %s fichier.jpeg\n", argv[0]);
	return EXIT_FAILURE;
    }
    const char *filename = argv[1];

    struct jpeg_desc *jdesc = read_jpeg(filename);

    struct bitstream *stream = get_bitstream(jdesc);

    int16_t bloc[64];
    read_bloc(stream, jdesc, bloc, 0, 0);
    int16_t new_bloc[64];

    d_zig_zag(bloc, new_bloc);

    for (int k = 0; k < 64; k++){
        printf("%hx, ", new_bloc[k]);
    }
    printf("\n");
    return EXIT_SUCCESS;
}
