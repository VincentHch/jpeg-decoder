#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>



# define PI		3.141592653 // pi



#include "jpeg_reader.h"
#include "huffman.h"
#include "bitstream.h"
#include "structure.h"
#include "decode_bloc.h"
#include "zig_zag.h"
#include "inverse_quantization.h"
#include "idct.h"



//merci https://hal.archives-ouvertes.fr/tel-01349553/document



uint8_t norme(float x)
{
	if (x > 255) {
		x = 255;
	}

	if (x < 0 ) {
		x = 0;
	}

	return (uint8_t) x;
}


void boitier(float i0, float i1, float *o0, float *o1, float k, uint8_t n)
    /* Codage du boitier de Loeffler */
{
    float x = ((float) n) * PI / 16;
    float a = k * cosf(x);
    float b = k * sinf(x);
    float partie_gauche = a * (i1 + i0);

    // Astuce pour gagner 2 multiplications par pixel
    *o0 = -(b + a)*i1 + partie_gauche;
    *o1 = (b - a)*i0 + partie_gauche;
}



void echangeur(float i0, float i1, float *o0, float *o1)
    /* Codage du circuit papillon de Loeffler */
{
    *o0 = i0 + i1;
    *o1 = i0 - i1;
}



void etape_4_3(float *ligne4, float *ligne3)
    /* Etape 4-->3 de l'algorithme de Loeffler */
{

    ligne3[0] = ligne4[0];
    ligne3[1] = ligne4[4];
    ligne3[2] = ligne4[2];
    ligne3[3] = ligne4[6];
    ligne3[5] = sqrt(2) * ligne4[3];
    ligne3[6] = sqrt(2) * ligne4[5];
    echangeur(ligne4[1], ligne4[7], &ligne3[7], &ligne3[4]);

}



void etape3_2(float *ligne3, float *ligne2)
    /* Etape 3-->2 de Loeffler */
{

    echangeur(ligne3[0], ligne3[1], &ligne2[0], &ligne2[1]);
    boitier(ligne3[2], ligne3[3], &ligne2[2], &ligne2[3], sqrt(2), 6);
    echangeur(ligne3[4], ligne3[6], &ligne2[4], &ligne2[6]);
    echangeur(ligne3[7], ligne3[5], &ligne2[7], &ligne2[5]);

}



void etape2_1(float *ligne2, float *ligne1)
    /* Etape 2-->1 de Loeffler */
{

    echangeur(ligne2[0], ligne2[3], &ligne1[0], &ligne1[3]);
    echangeur(ligne2[1], ligne2[2], &ligne1[1], &ligne1[2]);
    boitier(ligne2[4], ligne2[7], &ligne1[4], &ligne1[7], 1., 3);
    boitier(ligne2[5], ligne2[6], &ligne1[5], &ligne1[6], 1., 1);

}



void etape1_0(float *ligne1, float *ligne0)
    /* Etape 1--> 0 de Loeffler */
{

    echangeur(ligne1[0], ligne1[7], &ligne0[0], &ligne0[7]);
    echangeur(ligne1[1], ligne1[6], &ligne0[1], &ligne0[6]);
    echangeur(ligne1[2], ligne1[5], &ligne0[2], &ligne0[5]);
    echangeur(ligne1[3], ligne1[4], &ligne0[3], &ligne0[4]);

}



void loeffler(float *col_ou_ligne4, float *col_ou_ligne0)
{
    float col_ou_ligne3[8];
    float col_ou_ligne2[8];
    float col_ou_ligne1[8];

    etape_4_3(col_ou_ligne4, col_ou_ligne3);
    etape3_2(col_ou_ligne3, col_ou_ligne2);
    etape2_1(col_ou_ligne2, col_ou_ligne1);
    etape1_0(col_ou_ligne1, col_ou_ligne0);

}



void IDCT(int16_t bloc_freq[], uint8_t bloc_idct[])
    /* idct avec l'algorithme de Loeffler */
{
    float ligne4[8];
    float ligne0[8];
    float colonne4[8];
    float colonne0[8];
    float bloc_intermediaire[64];

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            ligne4[j] =  bloc_freq[8*i + j];
        }

        loeffler(ligne4, ligne0);

        for (int j = 0; j < 8; j++) {
            bloc_intermediaire[8*i + j] = ligne0[j];
        }
    }

    for (int i = 0; i < 8; i++) {

        for (int j = 0; j < 8; j++) {
            colonne4[j] = bloc_intermediaire[i + 8*j];
        }

        loeffler(colonne4, colonne0);

        for (int j = 0; j < 8; j++) {
            bloc_idct[i + 8*j] = norme(colonne0[j]/8. + 128.);
        }
    }
}



int test_loeffer(int argc, char **argv) //à changer en main pour tester loeffer
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
    int16_t bloc_izz[64];
    d_zig_zag(bloc, bloc_izz);

    printf("\n*****bloc 1 d_zig_zag*****\n");

    for (int i=0; i<64; i++){
        printf("%hx, ", bloc_izz[i]);
    }
    printf("\n");
    uint8_t bloc_idct[64];
    IDCT(bloc_izz, bloc_idct);

    printf("\n***** BLOC 1 idct *****\n");

    for (int i=0; i<64; i++){
      printf("%hhx, ", bloc_idct[i]);
    }
    printf("\n");


    read_bloc(stream, jdesc, bloc, 0 , valeur_prec);

    i_quantization(jdesc, bloc, 0);
    d_zig_zag(bloc, bloc_izz);
    printf("\n*****bloc 2 d_zig_zag*****\n");
    for (int i=0; i<64; i++){
        printf("%hx, ", bloc_izz[i]);
    }
    printf("\n");

    IDCT(bloc_izz, bloc_idct);

    printf("\n******BLOC 2 idct *******\n");
    for (int i=0; i<64; i++){
      printf("%hhx, ", bloc_idct[i]);
    }
    printf("\n");

    close_jpeg(jdesc);
    return 0;

}
