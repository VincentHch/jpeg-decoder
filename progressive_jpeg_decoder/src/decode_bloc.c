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
    uint8_t freq_debut = get_Ss(jpeg);
    uint8_t freq_fin = get_Se(jpeg);

    if (freq_debut == 0){
        bloc[0] = read_DC(stream, jpeg, indice_couleur, valeur_prec);
        freq_debut++;
    }
    read_AC(stream, jpeg, bloc, indice_couleur, freq_debut, freq_fin);
}


int16_t amplitude(int8_t m, uint32_t indice)
{
  //Amplitude en fonction de m et indice
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




void read_AC(struct bitstream *stream, struct jpeg_desc *jpeg, int16_t bloc[], uint8_t indice_couleur, uint8_t freq_debut, uint8_t freq_fin)
    /* On lie tout les AC d'un bloc. */
{
    uint8_t rle_code;
    uint8_t x_rle_m; // 2 octets contenants rle et m
    uint8_t m;
    uint8_t index_table = get_scan_component_huffman_index (jpeg , AC, indice_couleur);
    struct huff_table *h_table = get_huffman_table(jpeg, AC, index_table);

    uint8_t k;
    k = freq_debut;

    while (k <= freq_fin){

        x_rle_m = next_huffman_value(h_table, stream); //On décode
        rle_code =  x_rle_m / 16; //4 bit rle_code
        m = x_rle_m & 0x0F ; //4 bit magnitude

        if (rle_code == 0 && m == 0) { //Si fin de bloc on remplie avec des 0
            while (k <= freq_fin) {

                bloc[k] = 0;
                k++;
            }
            break;
        }

        while (rle_code != 0) { //On remplie avec des 0 les coefficients sautés.
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



