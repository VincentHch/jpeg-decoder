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

// merci https://hal.archives-ouvertes.fr/tel-01349553/document




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


/* On calcule l'idct naïve sur un bloc */

float coefficient(int c)
{
	if (c == 0) {
		return 1./sqrt(2);
	}

    else {
		return 1.;
	}
}

/* Ici on calcul l'idct S(x, y) */
int16_t idct_part(int x,int y,int16_t *bloc_freq, uint8_t debut, uint8_t fin)
{
	float S = 0;
    for (int k = debut; k <= fin; k++) {
        int i;
        int j;
        store_dezigzag_coordonnes(k, &i, &j);
		S = S + coefficient(i) * coefficient(j) *
		  cosf(((1. + 2*x) * i * (PI)) / 16.) *
		  cosf(((1. + 2*y) * j * (PI)) / 16.) *
		  (float)bloc_freq[8*i + j];
		}

    int16_t offset = (S/4.);
    if (debut == 0)
    {
        offset += 128;
    }
    //printf("x:%d, y:%d, offset:%d\n", x, y, offset);
	return offset;
}

void IDCT(int16_t bloc_freq[], uint8_t bloc_idct[], struct jpeg_desc *jpeg)
    /* Comme son nom l'indique c'est ici l'idct
     * codée de manière naïve avec n² opérations */
{
    uint8_t debut = get_Ss(jpeg);
    uint8_t fin = get_Se(jpeg);
	for (int i = 0; i < 8; i++) {
 		for (int j = 0; j < 8; j++) {
			bloc_idct[8*i+j] += idct_part(i,j,bloc_freq, debut, fin);
            bloc_idct[8*i+j] = norme(bloc_idct[8*i+j]);
		}
	}

}
