#ifndef __JPEG_DESC_H__
#define __JPEG_DESC_H__



#include <stdint.h>
#include <stdbool.h>



enum direction {
    DIR_H = 0,
    DIR_V = 1,
    /* sentinelle */
    DIR_NB
};



enum acdc {
    DC = 0,
    AC = 1,
    /* sentinelle */
    ACDC_NB
};



struct jpeg_desc; 



// general
extern struct jpeg_desc *read_jpeg(const char *filename);

extern void close_jpeg(struct jpeg_desc *jpeg);

extern const char *get_filename(struct jpeg_desc *jpeg);



// access to stream, placed just at the beginning of the scan raw data
extern struct bitstream *get_bitstream(struct jpeg_desc *jpeg);



// from DQT
extern uint8_t get_nb_quantization_tables(struct jpeg_desc *jpeg);

extern uint8_t *get_quantization_table(struct jpeg_desc *jpeg,
                                       uint8_t index);



// from DHT
extern uint8_t get_nb_huffman_tables(struct jpeg_desc *jpeg);

extern struct huff_table *get_huffman_table(struct jpeg_desc *jpeg,
                                            enum acdc acdc, uint8_t index);



// from Frame Header SOF0
extern uint16_t get_image_size(struct jpeg_desc *jpeg, enum direction dir);

extern uint8_t get_nb_components(struct jpeg_desc *jpeg);

extern uint8_t get_frame_component_sampling_factor(struct jpeg_desc *jpeg,
                                                   enum direction dir,
                                                   uint8_t frame_comp_index);

extern uint8_t get_frame_component_quant_index(struct jpeg_desc *jpeg,
                                               uint8_t frame_comp_index);

extern uint8_t get_scan_component_huffman_index(struct jpeg_desc *jpeg, enum acdc acdc, uint8_t scan_comp_index);



#endif
