#include "huffmantree.h"
#include <stdlib.h>
#include <stdio.h>

int insert_to_huffmantree( struct HuffmanNode* root, int data, int level )
{
	int ret = 0;
	if( root == NULL || root->saturated == SATURATED )
	{
		return -1;
	}
	
	
	if( root->left == NULL )
	{
		root->left = malloc( sizeof( struct HuffmanNode ) );
		root->left->left = NULL;
		root->left->right = NULL;
		root->left->saturated = NOT_SATURATED;
	}
	
	if( root->left->left != root->left )
	{
		if( level == 1 )
		{
			/*printf("\ninserted:%d",data);*/
			root->left->data = data;
			root->left->left = root->left->right = root->left;
			root->left->saturated = SATURATED;
			return 0;
		}
		else
		{
			if( NOT_SATURATED == root->left->saturated )
			{
				ret = insert_to_huffmantree( root->left, data, level - 1 );
				if( -1 == ret)
				{
					root->left->saturated = SATURATED;
				}
				else
				{
					return 0;
				}
			}
		}
		
	}
		
	if( root->right == NULL )
	{
		root->right = malloc( sizeof( struct HuffmanNode ) );
		root->right->left = NULL;
		root->right->right = NULL;
		root->right->saturated = NOT_SATURATED;
	}
	
	if( root->right->right != root->right )
	{
		if( level == 1 )
		{
			/*printf("\ninserted:%d",data);*/
			root->right->data = data;
			root->right->left = root->right->right = root->right;
			root->right->saturated = SATURATED;
			return 0;
		}
		else
		{
			if( NOT_SATURATED == root->right->saturated )
			{
				ret = insert_to_huffmantree( root->right, data, level - 1 );
				if( -1 == ret)
				{
					root->right->saturated = SATURATED;
				}
				else
				{
					return 0;
				}
			}
		}
		
	}	
		
	return -1;
}

void inflate( struct HuffmanNode* root, const char* bits, long int length )
{
	char bit;
	int i = 0;
	struct HuffmanNode* visitor = root;
	if( root == NULL )
		return;
	
	
	while( bit = bits[i++] )
	{
		if( bit == '0' )
			visitor = visitor->left;
		else
			visitor = visitor->right;
		
		if( visitor->left == visitor || visitor->right == visitor)
		{
			/*printf( "%d ",visitor->data );*/
			visitor = root;
		}
	}
	/*printf("\n");*/
}

void destroy_huffmantree( struct HuffmanNode* root )
{
	if( root == NULL )
		return;
	
	if( root->left )
	{
		if( root->left->left != root->left )
		{
			destroy_huffmantree( root->left );
		}
		else
		{
			free( root->left );
			root->left = NULL;
		}
	}
		
		
	if( root->right )
	{
		if( root->right->right != root->right )
		{
			destroy_huffmantree( root->right );
		}
		else
		{
			free( root->right );
			root->right = NULL;
		}
	}
	
	free( root );
	root = NULL;
	
	return;
		
}

void print_symbols( struct HuffmanNode* root )
{
	char bits[17];
	bits[0] = '\0';
	if( root == NULL )
		return;
	
	print_symbols_priv( root, bits, 1 );
}

void print_symbols_priv( struct HuffmanNode* root, char* bits, int level )
{
	if( root == NULL )
		return;
		
	if( root->left == root || root->right == root )
	{
		/*printf("\n\t%02x: %s", root->data, bits );*/
		return;
	}
	if( root->left )
	{
		bits[ level - 1 ] = '0';
		bits[ level ] = '\0';
		print_symbols_priv( root->left, bits, level + 1 );
	}
	
	if( root->right )
	{
		bits[ level - 1 ] = '1';
		bits[ level ] = '\0';
		print_symbols_priv( root->right, bits, level + 1 );
	}
}