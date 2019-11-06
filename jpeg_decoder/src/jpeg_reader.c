#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>



#include "bitstream.h"
#include "huffman.h"
#include "jpeg_reader.h"



struct jpeg_desc 
{
    /* structure du fichier qui contient
     * toutes les informations nécessaires */

    /* Informations générales */
    const char *filename; //nom du fichier
    struct bitstream *stream; //flux de bits

    /* en-tête DQT */
    uint8_t nombre_tables_quantification;
    uint8_t **tables_quantification;

    /* en-tête DHT */
    uint8_t nombre_tables_huffman;
    struct huff_table ***tables_huffman; //tableau de tableau de table de Huffman

    /* en-tête SOF0 */
    uint16_t taille_image[2];
    uint8_t nombre_composantes;
    uint16_t *component_id;
    uint8_t *echantillonage_h;
    uint8_t *echantillonage_v;
    uint8_t *indice_quant;

    /* en-tête SOS */
    uint8_t **indice_comp_huff;
};



struct jpeg_desc *jpeg_initial(const char *filename)
    /* on initialise notre image*/
{
    struct jpeg_desc *jpeg = malloc(sizeof(struct jpeg_desc));
    jpeg->filename = filename;
    jpeg->stream = create_bitstream(filename);

    jpeg->nombre_tables_quantification = 0;
    jpeg->tables_quantification = calloc(sizeof(uint8_t *), 4);

    jpeg->nombre_tables_huffman = 0;
    jpeg->tables_huffman = calloc(sizeof(struct huff_table **), 4);
    
    for (int k = 0; k < 4; k++) {
        (jpeg->tables_huffman)[k] = calloc(sizeof(struct huff_table *), 2);
    }

    jpeg->indice_comp_huff = calloc(sizeof(uint8_t *), 3);

    return jpeg;
}



void erreur(uint32_t nb_lu, uint32_t a_lire, char *msg)
    /* Affichage du message d'erreur */
{
    if (nb_lu != a_lire) {
        fprintf(stderr, "%s", msg);
        exit(1);
    }
}



void read_APPx(struct jpeg_desc *jpeg)
    /* Lecture de l'en-tête application data
     * le bitstream se trouve après le marqueur 0xffe0 */
{
    struct bitstream *stream = jpeg->stream;

    uint32_t longueur_section;
    uint8_t nb_lu;

    nb_lu = read_bitstream(stream, 16, &longueur_section, false);
    erreur(nb_lu, 16, "Erreur lecture bitstream AAPx\n");

    uint32_t valeur;

    nb_lu = read_bitstream(stream, 24, &valeur, false); //on lit JFIF\0
    
    erreur(nb_lu, 24, "Erreur lecture bitstream AAPx\n");

    if (valeur != 0x4a4649) { //il faut lire JFIF\0
        fprintf(stderr, "Erreur JFIF format\n");
        exit(1);
    }

    nb_lu = read_bitstream(stream, 16, &valeur, false); //on lit le reste de JFIF\0

    erreur(nb_lu, 16, "Erreur lecture bitstream AAPx\n");


    if (valeur != 0x4600) { //on lit la fin de JFIF\0
        fprintf(stderr, "Erreur JFIF format\n");
        exit(1);
    }

    longueur_section =longueur_section -  7;
    uint32_t bloc_a_lire = longueur_section / 4; //il faut terminer la lecture de l'en-tête
    uint32_t reste = longueur_section % 4;

    for (uint32_t k = 0; k < bloc_a_lire; k++) { //on lit les données sans les traiter
        nb_lu = read_bitstream(stream, 32, &valeur, false);
    
        erreur(nb_lu, 32, "Erreur lecture bitstream AAPx\n");
    }

    nb_lu = read_bitstream(stream, reste*8, &valeur, false); //on termine la lecture

    if (nb_lu != reste*8) {
        fprintf(stderr, "Erreur lecture bitstream AAPx\n");
        exit(1);
    }
}



void read_COM(struct jpeg_desc *jpeg)
    /* le prochain bit du stream est celui après le marqueur
     * 0xfffe, on écrira les commentaires, s'il y en a dans un fichier 
     * com.txt */
{
     struct bitstream *stream = jpeg->stream;

    uint32_t longueur_section;
    uint32_t nb_lu;

    nb_lu = read_bitstream(stream, 16, &longueur_section, false); //on lit la longueur de la section

    erreur(nb_lu, 16, "Erreur lecture COM\n");

    FILE* fichier = NULL;
    fichier = fopen("com.txt", "r+");

    if (fichier != NULL) {
        longueur_section = longueur_section - 2;

        for (uint32_t k = 0; k < longueur_section; k++) { //on parcourt la section
            uint32_t com;

            nb_lu = read_bitstream(stream, 8, &com, false);

            erreur(nb_lu, 8, "Erreur lecture COM\n"); //on écrit le commentaire dans le fichier

            fprintf(fichier, "%u", com);
        }
        fclose(fichier);
    }
}



void read_DQT(struct jpeg_desc *jpeg)
    /* le prochain bit du stream est celui après le marqueur
     * 0xffdb, il ne faut pas pas oublier que l'on peut avoir plusieurs DQT
     * ainsi on appelera cette fonction autant de fois que l'on a de sections
     * DQT */
{
    struct bitstream *stream = jpeg->stream;

    uint32_t longueur_section;
    uint32_t nb_lu;
    
    nb_lu = read_bitstream(stream, 16, &longueur_section, false);

    erreur(nb_lu, 16, "Erreur lecture DQT\n");

    jpeg->nombre_tables_quantification += 1; //on rentre le nombre de tables dans cette section

    uint32_t indice;
    nb_lu = read_bitstream(stream, 4, &indice, false); //c'est la précision inutile pour l'instant
    erreur(nb_lu, 4, "Erreur lecture DQT\n");

    nb_lu = read_bitstream(stream, 4, &indice, false); //on a l'indice de la table
    erreur(nb_lu, 4, "Erreur lecture DQT\n");

    (jpeg->tables_quantification)[indice] = calloc(sizeof(uint8_t), 64);

    for (int k = 0; k < 64; k++) { //on lit et remplis la table

        uint32_t valeur;
        nb_lu = read_bitstream(stream, 8, &valeur, false); //élement dans valeur
        erreur(nb_lu, 8, "Erreur lecture DQT\n");
        (jpeg->tables_quantification)[indice][k] = valeur;
    }
}



void read_SOF0(struct jpeg_desc *jpeg) 
    /* On lit l'en-tête SOF0 et on met les données utiles dans jpeg 
     *  le stream se trouve après le marqueur 0xffc0 */
{
    struct bitstream *stream = jpeg->stream;

    uint32_t longueur_section;
    uint32_t nb_lu;

    nb_lu = read_bitstream(stream, 16, &longueur_section, false); //on récupère la longueur de la section

    erreur(nb_lu, 16, "Erreur lecture bitstream SOF0\n");

    uint32_t precision;
    nb_lu = read_bitstream(stream, 8, &precision, false); //on récupère la précision même si l'on ne s'en sert pas encore

    erreur(nb_lu, 8, "Erreur lecture bitstream SOF0\n");

    uint32_t hauteur;
    uint32_t largeur;

    nb_lu = read_bitstream(stream, 16, &hauteur, false); //on récupère la hauteur
    erreur(nb_lu, 16, "Erreur lecture bitstream SOF0\n");
    (jpeg->taille_image)[1] = (uint16_t) hauteur;

    nb_lu = read_bitstream(stream, 16, &largeur, false); //on récupère la largeur
    erreur(nb_lu, 16, "Erreur lecture bitstream SOF0\n");
    (jpeg->taille_image)[0] = (uint16_t) largeur;

    uint32_t nombre_comp;
    
    nb_lu = read_bitstream(stream, 8, &nombre_comp, false); //on récupère le nombre de composantes de couleur
    jpeg->nombre_composantes = (uint8_t) nombre_comp;

    //on alloue les cases mémoires pour les tableaux de la structure
    jpeg->component_id = calloc(sizeof(uint16_t), 3);
    jpeg->echantillonage_h = calloc(sizeof(uint8_t), 3);
    jpeg->echantillonage_v = calloc(sizeof(uint8_t), 3);
    jpeg->indice_quant = calloc(sizeof(uint8_t), 3);

    erreur(nb_lu, 8, "Erreur lecture bitstream SOF0\n");

    for (uint32_t k = 0; k < nombre_comp; k++) {

        uint32_t id_comp;
        nb_lu = read_bitstream(stream, 8, &id_comp, false); //on récupère l'identifiant de la composante
        (jpeg->component_id)[k] = (uint8_t) id_comp;
        erreur(nb_lu, 8, "Erreur lecture bitstream SOF0\n");

        uint32_t sample_h;
        uint32_t sample_v;

        nb_lu = read_bitstream(stream, 4, &sample_h, false); //on récupère le facteur h d'échantillonnage
        erreur(nb_lu, 4, "Erreur lecture bitstream SOF0\n");
        (jpeg->echantillonage_h)[k] = (uint8_t) sample_h ;

        nb_lu =read_bitstream(stream, 4, &sample_v, false);  //on récupère le facteur v d'échantillonnage
        erreur(nb_lu, 4, "Erreur lecture bitstream SOF0\n");
        (jpeg->echantillonage_v)[k] = (uint8_t) sample_v;

        uint32_t id_quant;

        nb_lu = read_bitstream(stream, 8, &id_quant, false); //on récupère l'identifiant de la table de quantification pourr la composante de couleur
        erreur(nb_lu, 8, "Erreur lecture bitstream SOF0\n");
        (jpeg->indice_quant)[k] = (uint8_t) id_quant;

    }
}



void read_DHT(struct jpeg_desc *jpeg)
    /* On lit les tables de Huffman, le prochain bit
     * lu par le bitstream se trouve apres le marqueur 0xffc4 */
{
    struct bitstream *stream = jpeg->stream;

    uint32_t longueur_section;
    uint32_t nb_lu;
    nb_lu = read_bitstream(stream, 16, &longueur_section, false);
    erreur(nb_lu, 16, "Erreur lecture bitstream DHT\n");

    uint32_t inutile; //on y met les 3 bits non utilisés

    nb_lu = read_bitstream(stream, 3, &inutile, false); //on récupère 3 bits non utilisés qui doivent valoir 0
    erreur(nb_lu, 3, "Erreur lecture bitstream DHT\n");
    erreur(inutile, 0, "Erreur la valeur doit être 0 DHT\n");

    uint32_t type; //on y met le type de la table AC/DC
    nb_lu = read_bitstream(stream, 1, &type, false); //on a le type AD/DC
    erreur(nb_lu, 1, "Erreur lecture bitstream, DHT\n");

    uint32_t indice; //on y met l'indice de la table
    nb_lu = read_bitstream(stream, 4, &indice, false);
    erreur(nb_lu, 4, "Erreur lecture bitstream DHT\n");

    uint16_t nb_byte_read = 0;
      (jpeg->tables_huffman)[indice][type] = load_huffman_table(stream, &nb_byte_read); //on load la table de huffman
    longueur_section = longueur_section - 3 - nb_byte_read;
    erreur(longueur_section, 0, "Erreur lecture bitstream DHT\n");

    jpeg->nombre_tables_huffman += 1; //on oublie pas d'incrémenter le nombre de tables
}



void read_SOS(struct jpeg_desc *jpeg)
    /* On lit l'en-tête SOS, le prochain bit lu
     * par le bitstream se trouve après le marqueur 0xffda */
{
    struct bitstream *stream = jpeg->stream;

    uint32_t longueur_section;
    uint32_t nb_lu;

    nb_lu = read_bitstream(stream, 16, &longueur_section, false); //on lit la longueur de la section
    erreur(nb_lu, 16, "Erreur lecture bitstream SOS\n");

    uint32_t nombre_composantes;
    nb_lu = read_bitstream(stream, 8, &nombre_composantes, false); //on lit le nombre de composantes
    erreur(nb_lu, 8, "Erreur lecture bitstream SOS\n");

    for (int k = 0; k < (int) nombre_composantes; k++) {
        uint32_t indice_comp;
        nb_lu = read_bitstream(stream, 8, &indice_comp, false); //on récupère l'indice de composante
        erreur(nb_lu, 8, "Erreur lecture bitstream SOS\n");

        uint32_t indice_huff_DC; //indice dc de la table de huffman de la composante
        nb_lu = read_bitstream(stream, 4, &indice_huff_DC, false);
        erreur(nb_lu, 4, "Erreur lecture bitstream SOS\n");

        uint32_t indice_huff_AC; //indice ac de la table de huffman de la composante
        nb_lu = read_bitstream(stream, 4, &indice_huff_AC, false);
        erreur(nb_lu, 4, "Erreur lecture bitstream SOS\n");

        (jpeg->indice_comp_huff)[k] = calloc(sizeof(uint8_t), 2);
        (jpeg->indice_comp_huff)[k][0] = (uint8_t) indice_huff_DC; //on met DC à 0
        (jpeg->indice_comp_huff)[k][1] = (uint8_t) indice_huff_AC; //on met AC à 0

    
    }
    uint32_t progressif; //on s'occupe de ces données dans le décodeur progressif
        nb_lu = read_bitstream(stream, 24, &progressif, false);
        erreur(nb_lu, 24, "Erreur lecture bitstream SOS\n");
}



struct jpeg_desc *read_jpeg(const char *filename)
    /* On ouvre notre fichier jpeg et on
     * remplit la structure de données */
{
    struct jpeg_desc *jpeg  = jpeg_initial(filename);

    uint32_t marqueur = 0;
    uint32_t nb_lu;
    nb_lu = read_bitstream(jpeg->stream, 16, &marqueur, false);
    erreur(nb_lu, 16, "Erreur lecture entrée image\n");

    if (marqueur != 0xffd8) { //on a pas une image au format jpeg

        fprintf(stderr, "Image non compressée au format JPEG\n");
    }

    nb_lu = read_bitstream(jpeg->stream, 16, &marqueur, false);
    erreur(nb_lu, 16, "Erreur lecture marqueur APPx\n");

    while (marqueur != 0xffda) {
        switch (marqueur)
        {
            case 0xfffe:
                read_COM(jpeg);
                break;

            case 0xffe0:
                read_APPx(jpeg);
                break;

            case 0xffdb:
                read_DQT(jpeg);
                break;

            case 0xffc0:
                read_SOF0(jpeg);
                break;

            case 0xffc4:
                read_DHT(jpeg);
                break;

            default:
                fprintf(stderr, "Echec de lecture d'un marqueur\n");
                exit(1);
                break;
        }

        nb_lu = read_bitstream(jpeg->stream, 16, &marqueur, false); //on met à jour le marqueur
    }

    read_SOS(jpeg); //le prochain bit lu du bitstream est alors une donnée brut de l'image
    return jpeg;
}



const char *get_filename(struct jpeg_desc *jpeg)
    /* On renvoie le nom de l'image */
{
    return jpeg->filename;
}



struct bitstream *get_bitstream(struct jpeg_desc *jpeg)
    /* On renvoie le bitstream afin de lire
     * les données brutes */
{
    return jpeg->stream;
}



uint8_t get_nb_quantization_tables(struct jpeg_desc *jpeg)
    /* On renvoie le nombre de tables de
     * quantification */
{
    return jpeg->nombre_tables_quantification;
}



uint8_t *get_quantization_table(struct jpeg_desc *jpeg, uint8_t index)
    /* On retourne la table de quantification d'indice
     * index */
{
    return (jpeg->tables_quantification)[index];
}



uint8_t get_nb_huffman_tables(struct jpeg_desc *jpeg)
    /* On retourne le nombre de
     * tables de Huffman */
{
    return jpeg->nombre_tables_huffman;
}



struct huff_table *get_huffman_table(struct jpeg_desc *jpeg, enum acdc acdc, uint8_t index)
    /* On retourne la table de Huffman voulue */
{
    return (jpeg->tables_huffman)[index][acdc];
}



uint16_t get_image_size(struct jpeg_desc *jpeg, enum direction dir)
    /* On récupère la taille de l'image selon
     * la bonne direction. */
{
    return (jpeg->taille_image)[dir];
}



uint8_t get_nb_components(struct jpeg_desc *jpeg)
    /* On renvoie le nombre de composantes de 
     * couleur */
{
    return jpeg->nombre_composantes;
}



uint8_t get_frame_component_sampling_factor(struct jpeg_desc *jpeg, enum direction dir, uint8_t frame_comp_index)
    /* On renvoie le facteur d'échantillonnage voulus */
{
    switch (dir)
    {
        case(DIR_H):
            return (jpeg->echantillonage_h)[frame_comp_index];

        case(DIR_V):
            return (jpeg->echantillonage_v)[frame_comp_index];

        default:
            fprintf(stderr, "Direction mal rentrée\n");
            exit(1);
    }
}



uint8_t get_frame_component_quant_index(struct jpeg_desc *jpeg, uint8_t frame_comp_index)
    /* On renvoie l'index de la table de quantification 
     * associée à la composante de couleur */
{
    return (jpeg->indice_quant)[frame_comp_index];
}



uint8_t get_scan_component_huffman_index(struct jpeg_desc *jpeg, enum acdc acdc, uint8_t scan_comp_index)
    /* On renvoie l'index de la table de Huffman voulue */
{
    return (jpeg->indice_comp_huff)[scan_comp_index][acdc];
}



void close_jpeg(struct jpeg_desc *jpeg)
{
    for (int k = 0; k < 4; k++) {
        //libération des tables de huffman

        free_huffman_table((jpeg->tables_huffman)[k][0]);
        free_huffman_table((jpeg->tables_huffman)[k][1]);
        free((jpeg->tables_huffman)[k]);

        //libération tables de quantification
        free(jpeg->tables_quantification[k]);
    }
    for (int k = 0; k < 3; k++) {
        free((jpeg->indice_comp_huff)[k]);
    }

    free(jpeg->component_id);
    free(jpeg->indice_quant);
    free(jpeg->echantillonage_h);
    free(jpeg->echantillonage_v);
    free(jpeg->indice_comp_huff);
    free(jpeg->tables_quantification);
    free(jpeg->tables_huffman);

    close_bitstream(jpeg->stream);

    free(jpeg);
    
}

