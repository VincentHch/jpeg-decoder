#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "jpeg_reader.h"
#include "huffman.h"
#include "bitstream.h"
#include "structure.h"
#include "decode_bloc.h"


void read_bloc(struct bitstream *stream, struct jpeg_desc *jpeg, int16_t bloc[], uint8_t indice_couleur, int16_t valeur_prec)
    /* On lit entièrement un bloc: */
{
    bloc[0] = read_DC(stream, jpeg, indice_couleur, valeur_prec);
    read_AC(stream, jpeg, bloc, indice_couleur);

}


int16_t amplitude(int8_t m, uint32_t indice)
  /*Amplitude en fonction de m et indice */
{
    int16_t valeur;
    if (indice< (uint32_t) (1<<(m-1))){ //On est dans la moitié gauche
        valeur = -(1<<m)+1+indice;
    }
    else{ //Moitié droite
        valeur = indice;
    }
    return valeur;
}


int16_t read_DC(struct bitstream *stream, struct jpeg_desc *jpeg, uint8_t indice_couleur, int16_t valeur_prec)
    /* Retourne la valeur de DC */
{
    uint8_t index_table = get_scan_component_huffman_index(jpeg , DC, indice_couleur);

    struct huff_table *table = get_huffman_table(jpeg, DC, index_table);
    int8_t m = next_huffman_value(table, stream);

    uint32_t indice = 0;
    uint8_t nb_read = read_bitstream(stream, m, &indice, true);

    if (nb_read != m) {
        fprintf(stderr, "read_DC error");
        exit(3);
    }

    int16_t val = amplitude(m, indice) + valeur_prec;
    return val;
}




void read_AC(struct bitstream *stream, struct jpeg_desc *jpeg, int16_t bloc[], uint8_t indice_couleur)
    /* On lit tout les AC d'un bloc. */
{
    uint8_t rle_code;
    uint8_t x_rle_m; // 2 octets contenants rle et m
    uint8_t m;
    uint8_t index_table = get_scan_component_huffman_index (jpeg , AC, indice_couleur);
    struct huff_table *h_table = get_huffman_table(jpeg, AC, index_table);

    uint8_t k;
    k = 1;

    while (k < 64){

        x_rle_m = next_huffman_value(h_table, stream); //On décode
        rle_code =  x_rle_m / 16; //4 bit rle_code
        m = x_rle_m & 0x0F ; //4 bit magnitude

        if (rle_code == 0 && m == 0) { //Si fin de bloc on remplit avec des 0
            while (k < 64) {

                bloc[k] = 0;
                k++;
            }
            break;
        }

        while (rle_code != 0) { //On remplit avec des 0 les coefficients sautés.
            rle_code -= 1;
            bloc[k] = 0;
            k++;
        }

        if (m==0){ //Si la magnitude vaut 0 on met 0 (pas d'indice)
          bloc[k] = 0;
        }

        else{

          uint32_t indice;
          uint8_t nb_read = read_bitstream(stream, m, &indice, true);

          if (nb_read != m) {
              fprintf(stderr, "read_AC error\n");
              exit(3);
          }

          int16_t valeur = amplitude(m, indice);
          bloc[k] = valeur; //On rentre la valeur.
        }

        k++;
    }
}



int test_decode_bloc(int argc, char **argv) //A changer en main pour tester
    /* Test de decode bloc */
{
    if (argc != 2) {
	    fprintf(stderr, "Usage: %s fichier.jpeg\n", argv[0]);
	    return 0;
    }

    const char *filename = argv[1];

    struct jpeg_desc *jdesc = read_jpeg(filename);

    struct bitstream *stream = get_bitstream(jdesc);

    int16_t bloc[64];
    int16_t valeur_prec = 0;

    for (int i=0; i<10; i++){
      printf("\n****** BLOC %d ******", i);
      read_bloc(stream, jdesc, bloc, 0, valeur_prec);
      for (int i=0; i<64; i++){
        printf("%hx, ", bloc[i]);
      }
      printf("\n");
      valeur_prec = bloc[0];
    }

    close_jpeg(jdesc);
    return 0;
}
