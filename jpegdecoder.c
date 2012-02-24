#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "jpegdecoder.h"

#define BITS_PER_BYTE 8

byte bit_check[BITS_PER_BYTE] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};

byte zigzag[ MCU_BLOCK_SIZE ] = { 0,
								  1,8,
								  16,9,2,
								  3,10,17,24,
								  32,25,18,11,4,
								  5,12,19,26,33,40,
								  48,41,34,27,20,13,6,
								  7,14,21,28,35,42,49,56,
								  57,50,43,36,29,22,15,
								  23,30,37,44,51,58,
								  59,52,45,38,31,
								  39,46,53,60,
								  61,54,47,
								  55,62,
								  63 };
								  
double transform_matrix[ MCU_ROWS ][ MCU_COLUMNS ]= 
		  {
			  { 0.3536, 0.3536, 0.3536, 0.3536, 0.3536, 0.3536, 0.3536,  0.3536 },
			  { 0.4904, 0.4157, 0.2778, 0.0975, -0.0975, -0.2778, -0.4157, -0.4904 },
			  { 0.4619, 0.1913, -0.1913, -0.4619, -0.4619, -0.1913, 0.1913, 0.4619 },
			  { 0.4157, -0.0975, -0.4904, -0.2778, 0.2778, 0.4904, 0.0975, -0.4157 },
			  { 0.3536, -0.3536, -0.3536, 0.3536, 0.3536, -0.3536, -0.3536, 0.3536 },
			  { 0.2778, -0.4904, 0.0975, 0.4157, -0.4157, -0.0975, 0.4904, -0.2778 },
			  { 0.1913, -0.4619, 0.4619, -0.1913, -0.1913, 0.4619, -0.4619, 0.1913 },
			  { 0.0975, -0.2778, 0.4157, -0.4904, 0.4904, -0.4157, 0.2778, -0.0975 }
		  };
		  
double transform_matrix_transpose[ MCU_ROWS ][ MCU_COLUMNS ] =
		{
			{ 0.3536, 0.4904, 0.4619, 0.4157, 0.3536, 0.2778, 0.1913, 0.0975 },
			{ 0.3536, 0.4157, 0.1913, -0.0975, -0.3536, -0.4904, -0.4619, -0.2778 },
			{ 0.3536, 0.2778, -0.1913, -0.4904, -0.3536, 0.0975, 0.4619, 0.4157 },
			{ 0.3536, 0.0975, -0.4619, -0.2778, 0.3536, 0.4157, -0.1913, -0.4904 },
			{ 0.3536, -0.0975, -0.4619, 0.2778, 0.3536, -0.4157, -0.1913, 0.4904 },
			{ 0.3536, -0.2778, -0.1913, 0.4904, -0.3536, -0.0975, 0.4619, -0.4157 },
			{ 0.3536, -0.4157, 0.1913, 0.0975, -0.3536, 0.4904, -0.4619, 0.2778 },
			{ 0.3536, -0.4904, 0.4619, -0.4157, 0.3536, -0.2778, 0.1913, -0.0975 }
		};

int read_jpeg_file( const char* filename, struct RawImage** argPixmap )
{
	int ret = 0;
	int fc = 0;
	byte input = 0;
	
	int marker = 0;
	
	FILE* fp = NULL;
	long int filesize;
	
		
	struct HuffmanEncodingTable* huffmanencodingtable = NULL;
	
	struct QuantizationTable* quantizationtable = NULL;
	
	struct RawImage* pixmap = NULL;
	
	byte* scan = NULL;
	long int bytesscanned = 0;
	
	if( *argPixmap != NULL )
		return -1;
		
	pixmap = *argPixmap = malloc( sizeof( struct RawImage ) );
	if( pixmap == NULL )
		return -1;
		
		
		
	fp = fopen( filename, "rb" );
	
	if( NULL == fp )
		{
		return -1;
		}
	while( -1 != read_byte(fp, &input) )
	{
		if( ( marker_byte == input ) &&
			( -1 != read_byte(fp, &input) ) )
		{
			if( input >= app_0_byte && input <= app_f_byte )
			{
				/*printf("\n\nstart of application segment");*/
				handle_applicationsegment( fp, input & 0xF );
			}
			else
			{
				switch( input )
				{
					case soi_byte:
					{
						/*printf("start of image");*/
					}
					break;
					
					case eoi_byte:
					{
						/*printf("\n\nend of image\n\n");*/
					}
					break;
					
					case sof_byte:
					{
						/*printf("\n\nstart of frame");*/
						handle_frame( fp, pixmap );
					}
					break;
					
					case soh_byte:
					{
						/*printf("\n\nstart of huffman encoding table");*/
						handle_huffmanencoding_table( fp , &huffmanencodingtable );
					}
					break;
					
					case sos_byte:
					{
						/*printf("\n\nstart of scan");*/
						if( handle_scan( fp ) == 0 )
						{
							bytesscanned = scan_compressed_data( fp, &scan );
							/*printf( "\n\tbytesscanned:%d\n", bytesscanned );*/
							if( bytesscanned > 0 )
							{
								decode_scan( huffmanencodingtable, 
											 quantizationtable,
											 pixmap,
											 scan, 
											 bytesscanned );
							}
						}
					}
					break;
					
					case soq_byte:
					{
						/*printf("\n\nstart of quantization table");*/
						handle_quantization_table( fp, &quantizationtable );
					}
					break;
					
					case ignore_byte:
					{
						/*printf("ignore\n");*/
					}
					break;
					
					default:
					{
						/*printf("default 0x%02x\n", input );*/
					}
					break;
				}
			}
		}
		else
		{
			/*Do nothing*/
		}

	}
	
	/*printf("\n");*/
	
	if( scan )
	{
		free( scan );
	}
	fclose( fp );
	destroy_huffmanencoding_table( huffmanencodingtable );
	destroy_quantization_table( quantizationtable );
	return ret;
}

int handle_frame( FILE* fp, struct RawImage* pixmap )
{
	word length;
	byte sampleprecision;
	/*word imageheight;
	word imagewidth;*/
	byte numofcomponents;
	
	byte temp;
	byte index = 0;
	
	TRAP( error, read_word_bigendian( fp, &length ) )
	/*printf("\n\tlength:%d", length );*/
	TRAP( error, read_byte( fp, &sampleprecision ) )
	/*printf("\n\tsampleprecision:%d", sampleprecision );*/
	TRAP( error, read_word_bigendian( fp, &(pixmap->height) ) )
	/*printf("\n\timageheight:%d", pixmap->height );*/
	TRAP( error, read_word_bigendian( fp, &(pixmap->width) ) )
	/*printf("\n\timagewidth:%d", pixmap->width );*/
	TRAP( error, read_byte( fp, &numofcomponents ) )
	/*printf("\n\tnumofcomponents:%d", numofcomponents );*/
	while( index < numofcomponents )
	{
		/*printf("\n\tcomponent %d: ",index+1);*/
		TRAP( error, read_byte( fp, &temp ) )
		/*printf("%d ",temp);*/
		TRAP( error, read_byte( fp, &temp ) )
		/*printf("%x ",temp);*/
		TRAP( error, read_byte( fp, &temp ) )
		/*printf("%d ",temp);*/
		
		index++;
	}
	
	return 0;
	
	error:
		return -1;
}

int handle_quantization_table( FILE* fp, struct QuantizationTable** argtable )
{
	int type = 0;
	
	word length;
	
	byte* quantization_table = NULL;
	int quantization_table_length = 0;
	int macro_block_dim;
	
	int read = 0;
	
	byte ii = 0;
	
	
	struct QuantizationTable* table = NULL;
	
	if( 1 != read_word_bigendian( fp, &length ) )
	{
		/*printf("\n\tlength:%d", length );*/
		if( length != ( QUANTIZATION_BLOCK_SIZE + 3 ) )
			goto error;
	}
	
	if( EOF != ( type = fgetc(fp) ) )
	{
		/*printf("\n\ttype:%d\n", type );*/
	}
	
	if( *argtable == NULL )
	{
		*argtable = calloc( 1, sizeof( struct QuantizationTable ) );
		table = *argtable;
		table->type = (byte)type;
		table->quantization_matrix = malloc( sizeof( byte ) * QUANTIZATION_BLOCK_SIZE );
		table->link = NULL;
	}
	else
	{
		table = *argtable;
		while( table->link )
		{
			if( table->type == type )
				goto error;
			table = table->link;
		}
		table->link = calloc( 1, sizeof( struct QuantizationTable ) );
		
		table = table->link;
		table->type = (byte)type;
		table->quantization_matrix = malloc( sizeof( byte ) * QUANTIZATION_BLOCK_SIZE );
		table->link = NULL;
	}

	ii = 0;
	while( ii < QUANTIZATION_BLOCK_SIZE )
	{
		(table->quantization_matrix)[ zigzag[ ii ] ] 
				= ( ( read = fgetc( fp ) ) != EOF )?(byte)read : 0 ;
		if( read == EOF )
			goto error;
		ii++;
	}
	
/*	for( ii = 0; ii < QUANTIZATION_BLOCK_SIZE; ii++ )
	{
		if( !( ii % 8 ) )
			printf("\n");
		printf( "%2d ", (table->quantization_matrix)[ ii ] );
		
	}*/
	
	
	return 0;
	
error:
	return -1;
	
}

int handle_huffmanencoding_table( FILE* fp, struct HuffmanEncodingTable** argtable )
{
	word length;
	byte identifierinfo;
	
	byte bitspersymbol = 1;
	byte symbol = 0;
	byte index = 0;
	byte numsymbolsforbitscount[ MAX_HUFFMAN_BITS_PER_SYMBOL ];
	struct HuffmanEncodingTable* table = NULL;
	
	TRAP( error, read_word_bigendian( fp, &length ) )
	/*printf("\n\tlength:%d", length );*/
	
	TRAP( error, read_byte( fp, &identifierinfo ) )
	/*printf("\n\tidentifier info:%x", identifierinfo );*/
	
	if( *argtable == NULL )
	{
		*argtable = calloc( 1, sizeof( struct HuffmanEncodingTable ) );
		table = *argtable;
		table->identifier = identifierinfo;
		table->tree = calloc( 1, sizeof(struct HuffmanNode) );
		table->link = NULL;
	}
	else
	{
		table = *argtable;
		while( table->link )
		{
			if( table->identifier == identifierinfo )
				goto error;
			table = table->link;
		}
		
		table->link = calloc( 1, sizeof( struct HuffmanEncodingTable ) );
		table = table->link;
		table->link = NULL;
		
		table->identifier = identifierinfo;
		table->tree = calloc( 1, sizeof(struct HuffmanNode) );
		
	}
	
	while( bitspersymbol <= MAX_HUFFMAN_BITS_PER_SYMBOL )
	{
		TRAP( error, read_byte( fp, &numsymbolsforbitscount[bitspersymbol-1] ) )
		bitspersymbol++;
	}
		
	bitspersymbol = 1;
	while( bitspersymbol <= MAX_HUFFMAN_BITS_PER_SYMBOL )
	{
		index = 0;
		while( index < numsymbolsforbitscount[bitspersymbol-1] )
		{
			TRAP( error, read_byte( fp, &symbol ) )
			insert_to_huffmantree( table->tree, symbol, bitspersymbol );
			index++;
		}
		bitspersymbol++;
	}
	
	print_symbols( table->tree );
	
	return 0;
	
error:
	return -1;
}

int handle_scan( FILE* fp )
{
	word length;
	byte numofcomponentsinscan;
	byte index = 0;
	byte identifier;
	byte huffmantabletouse;
	byte ignore[3];
	byte input;
	
	FILE* fpw = NULL;
	
	
	TRAP( error, read_word_bigendian( fp, &length ) )
	/*printf("\n\tlength:%d", length );*/
	TRAP( error, read_byte( fp, &numofcomponentsinscan ) )
	/*printf("\n\tnumofcomponentsinscan:%d", numofcomponentsinscan );*/
	
	while( index < numofcomponentsinscan )
	{
		TRAP( error, read_byte( fp, &identifier ) )
		/*printf("\n\tidentifier:%d", identifier );*/
		TRAP( error, read_byte( fp, &huffmantabletouse ) )
		/*printf("\thuffmantabletouse:%02x", huffmantabletouse );*/
		index++;
	}
	TRAP( error, read_bytes( fp, ignore, 3 ) )
	
	
	/*fpw = fopen( "/home/sanil/test_bins.txt", "w+" );
	if( fpw == NULL )
		goto error;
	
	do
	{
		TRAP( success, read_byte( fp, &input ) )
		
		write_bin(fpw, input );
		fwrite( " ", 1,1,fpw);
			
	}while( 1 );
	
	
success:
	fclose( fpw );*/
	return 0;
	
error:
	return -1;
}

int handle_applicationsegment( FILE* fp, int number )
{
	word length;
	byte identifier[5];
	byte majorrevisionnum;
	byte minorrevisionnum;
	byte densityunit;
	word xdensity;
	word ydensity;
	byte thumbnailwidth;
	byte thumbnailheight;
	
	/*printf("\n\tnumber:%d", number );*/
	
	TRAP( error, read_word_bigendian( fp, &length ) )
	/*printf("\n\tlength:%d", length );*/
	
	if( 0 == number )
	{
		TRAP( error, read_bytes( fp, identifier, 5 ) )
		/*printf("\n\tidentifier:%s", identifier );*/
		TRAP( error, read_byte( fp, &majorrevisionnum ) )
		/*printf("\n\tmajorrevisionnum:%d", majorrevisionnum );*/
		TRAP( error, read_byte( fp, &minorrevisionnum ) )
		/*printf("\n\tminorrevisionnum:%d", minorrevisionnum );*/
		TRAP( error, read_byte( fp, &densityunit ) )
		/*printf("\n\tdensityunit:%d", densityunit );*/
		TRAP( error, read_word_bigendian( fp, &xdensity ) )
		/*printf("\n\txdensity:%d", xdensity );*/
		TRAP( error, read_word_bigendian( fp, &ydensity ) )
		/*printf("\n\tydensity:%d", ydensity );*/
		TRAP( error, read_byte( fp, &thumbnailwidth ) )
		/*printf("\n\tthumbnailwidth:%d", thumbnailwidth );*/
		TRAP( error, read_byte( fp, &thumbnailheight ) )
		/*printf("\n\tthumbnailheight:%d", thumbnailheight );*/
		
	}
	
	return 0;
	
error:
		return -1;	
}

int read_byte( FILE* fp, byte* arg )
{
	int ret = fgetc(fp);
	if( EOF != ret )
	{
		*arg = (byte)ret;
		return 0;
	}
	
	return -1;
}

int read_word( FILE* fp, word* arg )
{
	if( 1 != fread(arg,sizeof(word),1,fp) )
		return -1;
	return 0;
}

int read_doubleword( FILE* fp, doubleword* arg )
{
	if( 1 != fread(arg,sizeof(doubleword),1,fp) )
		return -1;
	return 0;
}

int read_word_bigendian( FILE* fp, word* arg )
{
	byte* temp = NULL;
	if( 1 != fread(arg,sizeof(word),1,fp) )
		return -1;
	temp = (byte*)arg;
	
	temp[0] ^= temp[1];
	temp[1] ^= temp[0];
	temp[0] ^= temp[1];
	
	return 0;
}

int read_doubleword_bigendian( FILE* fp, doubleword* arg )
{
	byte* temp = NULL;
	if( 1 != fread(arg,sizeof(doubleword),1,fp) )
		return -1;
	temp = (byte*)arg;
	
	temp[0] ^= temp[3];
	temp[3] ^= temp[0];
	temp[0] ^= temp[3];
	
	temp[1] ^= temp[2];
	temp[2] ^= temp[1];
	temp[1] ^= temp[2];
	
	return 0;
}

int read_bytes( FILE* fp, byte* arg, int count )
{
	if( count != fread(arg,sizeof(byte),count,fp) )
		return -1;
	return 0;
}

void destroy_huffmanencoding_table( struct HuffmanEncodingTable* table )
{
	struct HuffmanEncodingTable* temp = NULL;
	if( table == NULL)
		return;
		
	while( table )
	{
		destroy_huffmantree( table->tree );
		temp = table;
		table = table->link;
		free( temp );
	}
}

void write_bin( FILE* fpw, byte arg )
{
	int index = 0;
	char zero = '0';
	char one = '1';
	
	while ( index < 8 )
	{
		( arg<<index & 0x80 )?
			fwrite( &one, 1, 1, fpw ):
			fwrite( &zero, 1, 1, fpw );
		index++;
	}
}

long int scan_compressed_data( FILE* fp, byte** scan )
{
	if( *scan != NULL )
		return -1;
		
	byte* read;
	long int bytesscanned = 0L;
	long int curoffset = ftell( fp );
	long int filesize = 0L;
	fseek(fp, 0L, SEEK_END);
	filesize = ftell(fp);
	fseek(fp, curoffset, SEEK_SET);
	
	long int scansizemax = filesize - curoffset;
	int scan_end = 0;
	long int index = 0L;
	/*printf( "\n\tcuroffset:%d filesize:%d", curoffset, filesize );*/
	*scan = malloc( sizeof( byte ) * scansizemax );
	read = *scan;
	
	while( read_byte( fp, read ) != -1 )
	{
		if( *read == 0xFF )
		{
			if( read_byte( fp, read ) != -1 )
			{
				if( *read == 0x00 )
				{
					
					*read = 0xFF;
					//continue;
				}
				else
				{
					fseek(fp,-2L, SEEK_CUR );
					scan_end = 1;
					break;
				}
			}
		}
		read++;
		bytesscanned++;
	}
	
		
	return bytesscanned;
	
}

int decode_scan( struct HuffmanEncodingTable* huffmanencodingtable, 
				 struct QuantizationTable* quantizationtable,
				 struct RawImage* pixmap,
				 byte* scan, 
				 long int bytesscanned )
{
	/*FILE* fpw = NULL;*/
	long int index = 0;
	struct HuffmanEncodingTable* huffmanitertable = huffmanencodingtable;
	struct QuantizationTable* quantiter = quantizationtable;
	
	byte* Yquantization_matrix = NULL;
	byte* Cquantization_matrix = NULL;
	byte* current_quantization_matrix = NULL;
	
	struct HuffmanNode* Ydctree = NULL; 
	struct HuffmanNode* Yactree = NULL;
	struct HuffmanNode* Cbdctree = NULL;
	struct HuffmanNode* Cbactree = NULL;
	struct HuffmanNode* Crdctree = NULL; //own
	struct HuffmanNode* Cractree = NULL; //own
	
	struct HuffmanNode* node = NULL;
	struct HuffmanNode* rootnode = NULL;
	
	byte bitindex = 0;
	byte lookaheadbits = 0;
	
	short int prevYDC = 0;
	short int prevCbDC = 0;
	short int prevCrDC = 0;
	short int prevDC = 0;
	
	short int magnitude = 0;
	
	int ii = 0;
	int jj = 0;
	int kk = 0;
	
	double temp[ MCU_BLOCK_SIZE ];
	double value = 0;
	double realvalueY = 0;
	double realvalueCb = 0;
	double realvalueCr = 0;
	
	
	
		
	byte coeffsread = 0;
	byte zeros = 0;
	byte bitstoread = 0;
	
	word xPixels = 0;
	word yPixels = 0;
	
	
	short int YMcu_block[MCU_ROWS][MCU_COLUMNS];
	short int CbMcu_block[MCU_ROWS][MCU_COLUMNS];
	short int CrMcu_block[MCU_ROWS][MCU_COLUMNS];
	short int (*currentMcuBlock)[MCU_COLUMNS] = YMcu_block;
	
	double YMcu_Transform[MCU_ROWS][MCU_COLUMNS];
	double CbMcu_Transform[MCU_ROWS][MCU_COLUMNS];
	double CrMcu_Transform[MCU_ROWS][MCU_COLUMNS];
	
	byte mcu_RGB[ RGB_MCU_SIZE_IN_BYTES ];
	byte bb = 0;
	
	double redD, greenD, blueD;

	byte realindex = 0;
	
	int xstart = 0;
	int xend = 0;
	int ystart = 0;
	int yend = 0;
	int stridewidth = 0;
	int xcount = 0;
	int ycount = 0;
	
	if( !huffmanencodingtable || 
		!quantizationtable || 
		!pixmap || 
		!scan || 
		( bytesscanned <= 0 ) )
		goto error;
		
	/*fpw = fopen( "/home/sanil/test.txt", "w+" );
	if( !fpw )
		goto error;*/

	if( pixmap->height > 0 && 
		pixmap->width > 0 )
	{
		stridewidth = pixmap->width * RGB_PIXEL_SIZE_IN_BYTES;
		pixmap->data = malloc(  stridewidth * pixmap->height );
		yend = ystart + MCU_HEIGHT;
		if( yend > pixmap->height )
			yend = pixmap->height;
	}
	else
		goto error;
		
	while( huffmanitertable )
	{
		switch( huffmanitertable->identifier )
		{
			case 0x00:
				Ydctree = huffmanitertable->tree;
				break;
			case 0x01:
				Cbdctree = huffmanitertable->tree;
				Crdctree = malloc( sizeof( struct HuffmanNode ) );
				Crdctree->data = Cbdctree->data;
				Crdctree->saturated = Cbdctree->saturated;
				Crdctree->left = Cbdctree->left;
				Crdctree->right = Cbdctree->right;
				break;
			case 0x10:
				Yactree = huffmanitertable->tree;
				break;
			case 0x11:
				Cbactree = huffmanitertable->tree;
				Cractree = malloc( sizeof( struct HuffmanNode ) );
				Cractree->data = Cbactree->data;
				Cractree->saturated = Cbactree->saturated;
				Cractree->left = Cbactree->left;
				Cractree->right = Cbactree->right;
				break;
				
		}
		huffmanitertable = huffmanitertable->link;
	}
	rootnode = node = Ydctree;
	
	while( quantiter )
	{
		if( quantiter->type == 0)
			Yquantization_matrix = quantiter->quantization_matrix;
		else
			Cquantization_matrix = quantiter->quantization_matrix;
		quantiter = quantiter->link;
	}
	current_quantization_matrix = Yquantization_matrix;

	
	
	coeffsread = 0;

	memset(YMcu_block, 0, sizeof(YMcu_block[0][0]) * MCU_ROWS * MCU_COLUMNS );
	memset(CbMcu_block, 0, sizeof(CbMcu_block[0][0]) * MCU_ROWS * MCU_COLUMNS );
	memset(CrMcu_block, 0, sizeof(CrMcu_block[0][0]) * MCU_ROWS * MCU_COLUMNS );
	currentMcuBlock = YMcu_block;
	
	while( index < bytesscanned )
	{
		bitindex = 0;
		
		while( bitindex < BITS_PER_BYTE )
		{
			if( coeffsread >= MCU_BLOCK_SIZE )
			{
				coeffsread = 0;
				if( currentMcuBlock == CrMcu_block )
				{
					
					memset ( mcu_RGB, 0, sizeof( byte ) * RGB_MCU_SIZE_IN_BYTES );
					bb = 0;
					
					for( ii = 0; ii < MCU_ROWS; ii++ )
					{
						for( jj = 0; jj < MCU_COLUMNS; jj++ )
						{
							YMcu_Transform[ii][jj] = 0;
							CbMcu_Transform[ii][jj] = 0;
							CrMcu_Transform[ii][jj] = 0;
							for( kk = 0; kk < MCU_COLUMNS; kk++ )
							{
								YMcu_Transform[ii][jj] += YMcu_block[ii][kk] * transform_matrix[kk][jj];
								CbMcu_Transform[ii][jj] += CbMcu_block[ii][kk] * transform_matrix[kk][jj];
								CrMcu_Transform[ii][jj] += CrMcu_block[ii][kk] * transform_matrix[kk][jj];
							}
						}
					}
					
					
					for( ii = 0; ii < MCU_ROWS; ii++ )
					{
						for( jj = 0; jj < MCU_COLUMNS; jj++ )
						{
							realvalueY = 0;
							realvalueCb = 0;
							realvalueCr = 0;
							for( kk = 0; kk < MCU_COLUMNS; kk++ )
							{
								realvalueY += transform_matrix_transpose[ii][kk] * YMcu_Transform[kk][jj];
								realvalueCb += transform_matrix_transpose[ii][kk] * CbMcu_Transform[kk][jj];
								realvalueCr += transform_matrix_transpose[ii][kk] * CrMcu_Transform[kk][jj];
								
							}
							YMcu_block[ii][jj] = (short int )nearbyint( realvalueY ) + 128;
							CbMcu_block[ii][jj] = (short int )nearbyint( realvalueCb ) + 128;
							CrMcu_block[ii][jj] = (short int )nearbyint( realvalueCr ) + 128;
							
							YMcu_block[ii][jj] = YMcu_block[ii][jj] > 255 ? 255 :
												 YMcu_block[ii][jj] < 0 ? 0 :
												 YMcu_block[ii][jj] ;
												 
							CbMcu_block[ii][jj] = CbMcu_block[ii][jj] > 255 ? 255 :
												 CbMcu_block[ii][jj] < 0 ? 0 :
												 CbMcu_block[ii][jj] ;
							
							CrMcu_block[ii][jj] = CrMcu_block[ii][jj] > 255 ? 255 :
												 CrMcu_block[ii][jj] < 0 ? 0 :
												 CrMcu_block[ii][jj] ;
												 
							//redD = nearbyint( YMcu_block[ii][jj] + 1.402*CrMcu_block[ii][jj] );
							mcu_RGB[bb++] = clip(( 298 * ( YMcu_block[ii][jj] - 16 )           + 409 * ( CrMcu_block[ii][jj] - 128 ) + 128) >> 8);
							mcu_RGB[bb++] = clip(( 298 * ( YMcu_block[ii][jj] - 16 ) - 100 * ( CbMcu_block[ii][jj] - 128 ) - 208 * ( CrMcu_block[ii][jj] - 128 ) + 128) >> 8);
							mcu_RGB[bb++] = clip(( 298 * ( YMcu_block[ii][jj] - 16 ) + 516 * ( CbMcu_block[ii][jj] - 128 )           + 128) >> 8);
							
							/*mcu_RGB[bb] = redD > 255.0 ? 255 :
											redD < 0.0 ? 0 :
											(byte)redD;
							//fprintf( fpw, "%3d,", mcu_RGB[bb]);
							bb++;
							
							greenD = nearbyint( YMcu_block[ii][jj] 
												- 0.34414*CbMcu_block[ii][jj]  
												- 0.71414*CrMcu_block[ii][jj] );
							mcu_RGB[bb] = greenD > 255.0 ? 255 :
											greenD < 0.0 ? 0 :
											(byte)greenD;
							//fprintf( fpw, "%3d,", mcu_RGB[bb]);
							bb++;
							
							blueD = nearbyint( YMcu_block[ii][jj] 
											   + 1.772*CbMcu_block[ii][jj] );
							mcu_RGB[bb] = blueD > 255.0 ? 255 :
											blueD < 0.0 ? 0 :
											(byte)blueD;
							//fprintf( fpw, "%3d ", mcu_RGB[bb]);
							bb++;*/
							
							
							
						}
						/*fprintf( fpw, "\n");*/
					}
					
					/*fprintf( fpw, "\n");*/
					
					if( yend >= pixmap->height && xend >= stridewidth )
						goto error;
						
					if( xend >= stridewidth )
					{
						xstart = 0;
						xend = xstart + MCU_STRIDE_WIDTH;
						if( xend > stridewidth )
							xend = stridewidth;
							
						ystart += MCU_HEIGHT;
						yend = ystart + MCU_HEIGHT;
						if( yend > pixmap->height )
							yend = pixmap->height;
					}
					else if( xstart < stridewidth )
					{
						xend = xstart + MCU_STRIDE_WIDTH;
						if( xend > stridewidth )
							xend = stridewidth;
					}
					else
						goto error;
						
					ii = 0;
					ycount = yend - ystart;
					xcount = xend - xstart;
					
					while( ii < ycount )
					{
						memcpy( pixmap->data + ( ystart + ii )*stridewidth + xstart, 
								mcu_RGB + ii * MCU_STRIDE_WIDTH, 
								xcount );
						ii++;
					}
					
					xstart += MCU_STRIDE_WIDTH;
					
					memset(YMcu_block, 0, sizeof(YMcu_block[0][0]) * MCU_ROWS * MCU_COLUMNS );
					memset(CbMcu_block, 0, sizeof(CbMcu_block[0][0]) * MCU_ROWS * MCU_COLUMNS );
					memset(CrMcu_block, 0, sizeof(CrMcu_block[0][0]) * MCU_ROWS * MCU_COLUMNS );	
				}
				
				currentMcuBlock = currentMcuBlock == YMcu_block ? CbMcu_block :
								  currentMcuBlock == CbMcu_block ? CrMcu_block :
								  currentMcuBlock == CrMcu_block ? YMcu_block :
								  currentMcuBlock;
								  
			
				
			}
			
			node = *scan & bit_check[ bitindex++ ]?
				node->right : node->left;
				
			if( node == NULL )
			{
				/*printf("\nerror\n");*/
				goto error;
			}
			else if( node->left == node &&  node->right == node ) //leaf
			{
				
				if( rootnode == Ydctree || 
					rootnode == Cbdctree ||
					rootnode == Crdctree ) // dc tree
				{
					lookaheadbits = 0;
					magnitude = 0;
					while( lookaheadbits < node->data )
					{
						while(  ( bitindex < BITS_PER_BYTE ) && 
								( lookaheadbits < node->data ) )
						{
							magnitude = magnitude << 1;
							if( *scan & bit_check[ bitindex++ ] )
								magnitude++;
							lookaheadbits++;	
						}
						if( lookaheadbits < node->data )
						{
							bitindex = 0;
							scan++;
							index++;
						}
					}
					
					prevDC = rootnode == Ydctree ? prevYDC :
							 rootnode == Cbdctree ? prevCbDC :
							 rootnode == Crdctree ? prevCrDC :
							 prevDC;
					
					
					magnitude = extend( magnitude, node->data );
					//fprintf( fpw, "\n\nDC: b:%d m:%d p:%d r:%d", node->data, magnitude, prevDC, magnitude + prevDC );
					if( coeffsread != 0 )
						goto error;
					currentMcuBlock[0][0] = current_quantization_matrix[coeffsread] * ( magnitude + prevDC );
										
					coeffsread++;
					
					prevYDC = rootnode == Ydctree ? magnitude + prevDC : prevYDC;
					prevCbDC = rootnode == Cbdctree ? magnitude + prevDC : prevCbDC;
					prevCrDC = rootnode == Crdctree ? magnitude + prevDC : prevCrDC;
					
				}
				else // ac tree
				{
					
					if( node->data == 0x00 )
					{
						coeffsread = MCU_BLOCK_SIZE;
						//fprintf( fpw, "\nAC:EOB" );
					}
					else if( node->data == 0xF0 )
					{
						coeffsread += 16;
						//fprintf( fpw, "\nAC:16z" );
					}
					else
					{
						zeros = ( node->data & 0xF0 ) >> 4;
						bitstoread = node->data & 0x0F;
						coeffsread += zeros;
						lookaheadbits = 0;
						magnitude = 0;
						while( lookaheadbits < bitstoread )
						{
							while(  ( bitindex < BITS_PER_BYTE ) && 
								( lookaheadbits < bitstoread ) )
							{
								magnitude = magnitude << 1;
								if( *scan & bit_check[ bitindex++ ] )
									magnitude++;
								lookaheadbits++;	
							}
							if( lookaheadbits < bitstoread )
							{
								bitindex = 0;
								scan++;
								index++;
							}
						}
						magnitude = extend( magnitude, bitstoread );
						if( coeffsread >= MCU_BLOCK_SIZE )
							goto error;
						realindex = zigzag[ coeffsread ];	
						currentMcuBlock[realindex/MCU_COLUMNS][realindex%MCU_COLUMNS] = current_quantization_matrix[ zigzag[ coeffsread ] ] * magnitude;
						coeffsread++;
						//fprintf( fpw, "\nAC:z:%d m:%d c:%d", zeros, magnitude, coeffsread );
						
					}
					
					
				}
				
				current_quantization_matrix 
					= ( rootnode == Yactree  && coeffsread >= MCU_BLOCK_SIZE ) ? Cquantization_matrix :
					  ( rootnode == Cbactree  && coeffsread >= MCU_BLOCK_SIZE ) ? Cquantization_matrix :
					  ( rootnode == Cractree  && coeffsread >= MCU_BLOCK_SIZE ) ? Yquantization_matrix :
					  current_quantization_matrix;
											
				rootnode = node = 
						   ( rootnode == Ydctree ) ? Yactree : 
						   ( rootnode == Yactree  && coeffsread >= MCU_BLOCK_SIZE ) ? Cbdctree :
						   ( rootnode == Cbdctree ) ? Cbactree : 
						   ( rootnode == Cbactree  && coeffsread >= MCU_BLOCK_SIZE ) ? Crdctree :
						   ( rootnode == Crdctree ) ? Cractree : 
						   ( rootnode == Cractree  && coeffsread >= MCU_BLOCK_SIZE ) ? Ydctree :
						   rootnode;
								  
				
			}
			else
			{
				//Do nothing
			}
		}
		
		scan++;
		index++;
		
	}
	
	/*if( fpw )
	{
		fclose( fpw );
	}*/
	if( Crdctree )
	{
		free( Crdctree );
	}
	if( Cractree )
	{
		free( Cractree );
	}
	return 0;
	
error:
	/*if( fpw )
	{
		fclose( fpw );
	}*/
	if( Crdctree )
	{
		free( Crdctree );
	}
	if( Cractree )
	{
		free( Cractree );
	}
	return -1;
}

int extend( int magnitude, byte bits )
{
	int check = 0;
	if( bits > 0 && bits <= 11 )
	{
		check = 1 << ( bits - 1 );
		if( ! ( magnitude & check ) && ( magnitude < check ) )
			magnitude += 1 + ( -1<<bits ) ;	
	}
	
	return magnitude;
}

void destroy_quantization_table( struct QuantizationTable* table )
{
	struct QuantizationTable* temp = NULL;
	if( table == NULL)
		return;
		
	while( table )
	{
		temp = table;
		table = table->link;
		free( temp->quantization_matrix );
		free( temp );
	}
	
}

void destroy_mcu_blocks( struct MCUBlock* block )
{
	struct MCUBlock* temp = NULL;
	if( block == NULL)
		return;
		
	while( block )
	{
		temp = block;
		free( temp->coeffs );
		free( temp );
		block = block->link;
	}
}

byte clip( double d )
{
	double nd = nearbyint( d );
	return  nd > 255.0 ? 255 :
			nd < 0.0 ? 0 :
			(byte)nd;
}