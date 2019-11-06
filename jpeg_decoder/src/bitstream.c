#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>



struct bitstream
     /*structure pour représenter le flux de bits d'un fichier*/
{
    FILE *fp;//fichier actuel

    uint8_t octet_courant; // octet en lecture en ce moment

    int indice_lecture; // indice du dernier bit lu dans l'octet + 1 (ou premier bit a lire)

    uint16_t est_ce_fini; //stocke la valeur des 4 derniers octets où l'on est passé
};



//------------------------------------------------------------------------------------------------
// Ouverture et fermeture du bitstream
// -----------------------------------------------------------------------------------------------



struct bitstream *create_bitstream(const char *filename)
    /*initialise la bitestream liée à *filename*/
{
    struct bitstream *stream = malloc(sizeof(struct bitstream));

    if (stream == NULL) {
        fprintf(stderr, "bitstream erroné");
        exit(0);
    }

    else {
        stream->indice_lecture = 0;

        stream->fp = fopen(filename,"rb");

        if (stream->fp != NULL) { //si le fichier exite
            fread(&(stream->octet_courant), 1, 1, stream->fp);
            stream-> est_ce_fini = 0;
            return stream;
        }
    }

    fprintf(stderr, "Fichier %s introuvable\n", filename);
    exit(0);
}



void close_bitstream(struct bitstream *stream)
    /*ferme la fichier et free la memoire*/
{
    fclose(stream->fp);
    free(stream);
}



//------------------------------------------------------------------------------------------------
//indicateur de fin de fichier
//------------------------------------------------------------------------------------------------



bool end_of_bitstream(struct bitstream *stream)
{
     if (stream == NULL) {
        fprintf(stderr, "bitstream erroné");
        exit(0);
    }

     if (stream->est_ce_fini == 0xffd9) {
        return true;
   }

   return false;
}



//------------------------------------------------------------------------------------------------
//Lecture du  bitstream
//------------------------------------------------------------------------------------------------



uint8_t read_bitstream(struct bitstream *stream, uint8_t nb_bits, uint32_t *dest, bool discard_byte_stuffing)
{
    *dest = 0;

    if (stream->fp == NULL) {
        fprintf(stderr, "Erreur dans l'ouverture du fichier");
        exit(0);
    }

    else if (nb_bits>32) {
        fprintf(stderr, "Nombre de bits à lire supérieur à 32");
        exit(0);
    }

    else {
        int nb_bit_lus = 0;
        uint8_t bit_lu;
        while (nb_bits>0) {
            if (stream->indice_lecture == 8) { //fin octet->recomencer lecture à 0 sur octet suivant
                stream->indice_lecture = 0;
                fread(&(stream->octet_courant), 1, 1, stream->fp);
                if (discard_byte_stuffing == true && ((stream->est_ce_fini & 0x00ff) ==0xff) && stream->octet_courant == 0x00) {
                    fread(&(stream->octet_courant), 1, 1, stream->fp); //si byte_stuffing on ignore cet octet, on passe au suivant
                    stream->indice_lecture = 0;
                }
            }

            bit_lu = (stream->octet_courant & (0b1<<(7-stream->indice_lecture)))>>(7 - stream->indice_lecture); //valeur du bit lu

            *dest = *dest << 1;
            *dest += bit_lu; //on l'ajoute a l'ensemble des bits deja lus

            stream->est_ce_fini = (stream->est_ce_fini << 1) + bit_lu; //on garde la valeur des 4 derniers bits lus updated
            stream->indice_lecture += 1;//la prochaine lecture se fait a l'indice suivant

            nb_bits--;
            nb_bit_lus++;
        }

    return nb_bit_lus;
    }
}
