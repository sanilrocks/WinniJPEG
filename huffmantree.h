#ifndef __HUFFMAN_TREE_H__
#define __HUFFMAN_TREE_H__

#define NOT_SATURATED 0
#define SATURATED 1

struct HuffmanNode
{
	int data;
	char saturated;
	struct HuffmanNode* left;
	struct HuffmanNode* right;
};

int insert_to_huffmantree( struct HuffmanNode* root, int data, int level );

void inflate( struct HuffmanNode* root, const char* bits, long int length );
void destroy_huffmantree( struct HuffmanNode* root );

void print_symbols( struct HuffmanNode* root );

void print_symbols_priv( struct HuffmanNode* root, char* bits, int level );

/*void inflate( )*/

#endif //__HUFFMAN_TREE_H__
