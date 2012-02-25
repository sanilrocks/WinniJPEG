#ifndef __JPEG_DECODER_H__
#define __JPEG_DECODER_H__

#define MAX_HUFFMAN_BITS_PER_SYMBOL 16
#define MCU_BLOCK_SIZE 64
#define MCU_ROWS 8
#define MCU_COLUMNS 8
#define QUANTIZATION_BLOCK_SIZE 64

#define RGB_PIXEL_SIZE_IN_BYTES 3
#define MCU_STRIDE_WIDTH 24	//RGB_PIXEL_SIZE_IN_BYTES * MCU_COLUMNS
#define MCU_HEIGHT 8

#define RGB_MCU_SIZE_IN_BYTES 192	// MCU_BLOCK_SIZE * RGB_PIXEL_SIZE_IN_BYTES

#define TRAP( label, x ) do { if( -1 == x ) \
                              { \
                                  goto label; \
                              } }while(0); \

#define NOT_SATURATED 0
#define SATURATED 1
                              

typedef unsigned char byte;
typedef unsigned short int word;
typedef unsigned long int doubleword;

enum jpeg_marker {
    ignore_byte = 0x00,		/* ignore byte */
    sof_byte = 0xc0,		/* start of frame */
    soh_byte = 0xc4,		/* start of huffman encoding table */

    soi_byte = 0xd8,		/* start of image */
    eoi_byte = 0xd9,		/* end of image */
    sos_byte = 0xda,		/* start of scan */
    soq_byte = 0xdb,		/* start of quantization table */

    app_0_byte = 0xe0,		/* start of application segment 0 */
    app_f_byte = 0xef,		/* start of application segment f */

    marker_byte = 0xff		/* marker */
};

struct HuffmanEncodingTable {
    byte identifier;
    struct HuffmanNode *tree;
    struct HuffmanEncodingTable *link;
};

struct QuantizationTable {
    byte type;
    byte *quantization_matrix;
    struct QuantizationTable *link;
};


struct MCUBlock {
    short int coeffs[MCU_ROWS][MCU_COLUMNS];
    struct MCUBlock *link;
};

struct RawImage {
    word height;
    word width;
    byte *data;
};

struct HuffmanNode {
    int data;
    char saturated;
    struct HuffmanNode *left;
    struct HuffmanNode *right;
};

int insert_to_huffmantree(struct HuffmanNode *root, int data, int level);

void destroy_huffmantree(struct HuffmanNode *root);


int read_jpeg_file(const char *filename, struct RawImage **argPixmap);

int handle_frame(FILE * fp, struct RawImage *pixmap);
int handle_quantization_table(FILE * fp,
			      struct QuantizationTable **argtable);
int handle_huffmanencoding_table(FILE * fp,
				 struct HuffmanEncodingTable **argtable);
int handle_scan(FILE * fp);
int handle_applicationsegment(FILE * fp, int number);


int read_byte(FILE * fp, byte * arg);
int read_word(FILE * fp, word * arg);
int read_word_bigendian(FILE * fp, word * arg);
int read_doubleword(FILE * fp, doubleword * arg);
int read_doubleword_bigendian(FILE * fp, doubleword * word);

int read_bytes(FILE * fp, byte * arg, unsigned int count);

void destroy_huffmanencoding_table(struct HuffmanEncodingTable *table);
void destroy_quantization_table(struct QuantizationTable *table);
void destroy_mcu_blocks(struct MCUBlock *block);

void write_bin(FILE * fpw, byte arg);

long int scan_compressed_data(FILE * fp, byte ** scan);

int decode_scan(struct HuffmanEncodingTable *huffmanencodingtable,
		struct QuantizationTable *quantizationtable,
		struct RawImage *pixmap,
		byte * scan, long int bytesscanned);

int extend(int magnitude, byte bits);

byte clip(double d);


#endif				//__JPEG_DECODER_H__
