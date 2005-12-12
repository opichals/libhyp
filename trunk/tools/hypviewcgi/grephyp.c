/*
 * libhyp: ST-Guide HYPertext file handling library
 * Copyright (c) 2005 Standa Opichal / JAY Software
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * $Source: /local/libhyp.cvs/libhyp/tools/hypviewcgi/grephyp.c,v $
 *  
 * CVS info:
 *   $Author: standa $
 *   $Date: 2005-12-12 17:55:10 $
 *   $Revision: 1.2 $
 */

#include <stdio.h>
#include <sys/fcntl.h>

#include <libhyp.h>

#define LN_SIZE 254

void emit_quoted( char *s )
{
	char *t;
	while( t = (char*)strpbrk( s, "<>&\'\"" ) ) {
		char c = *t;
		*t = '\0';
		printf( "%s", s );
		switch ( c ) {
			case '<': printf( "&lt;" ); break;
			case '>': printf( "&gt;" ); break;
			case '&': printf( "&amp;" ); break;
			case '\'': printf( "&apos;" ); break;
			case '\"': printf( "&quot;" ); break;
		}
		*t = c;
		s = t+1;
	}
	printf( "%s", s );
}

int search_node( HYP *hyp, int index, const char *s )
{
	HYP_NODE *node = hyp_parse_node( hyp, index);
	if ( node ) {
		char ln[LN_SIZE+2]; /* + '\n' + '\0' */
		size_t tlen = 1;
		char *t, *lnEnd;
		int first = 1;
		int line = -1;
		HYP_HDOC_IDXITEM *ie = &hyp->index_table[ index ];
		HYP_ITEM *item;


		/* search in the node name and title first */
		strncpy( ln, node->name, LN_SIZE );
		if ( node->title ) {
			tlen = strlen( ln);
			if ( tlen < LN_SIZE ) {
				ln[tlen] = ' ';
				strncpy( &ln[tlen+1], node->title, LN_SIZE-tlen-1 );
			}
		}
		strcat( ln, "\n");
		t = ln;
	       	tlen = strlen( t);


		item = hyp_node_item_first( node );
		while ( item ) {
			/* if the line is complete */
			if ( t && t[ tlen - 1 ] == '\n' ) {
				if ( strstr( ln, s) ) {
					char lnum[10];
					sprintf( lnum, "%5d: ", line);

					if ( first ) {
						printf( "\n");
						printf( "<!--a href=\"index=%d&line=%d\"-->", index, line);
						emit_quoted( node->title ? node->title : node->name);
						printf( "<!--/a-->\n");
						first = 0;
					}
					if ( line > -1 ) { /* line == -1 -> search in the title and name */
						emit_quoted( lnum );
						emit_quoted( ln);
					}
				}
				line++;
				lnEnd = ln;
			}

			switch ( item->type ) {
				case HYPT_TEXT:
					t = ((HYP_TEXT*)item)->string;
					break;
				case HYPT_LINK:
					t = ((HYP_LINK*)item)->destination;
					break;
				default:
					t = NULL;
			}

			if ( t ) {
				/* concat to the line */
				tlen = strlen( t);
				strncpy( lnEnd, t, LN_SIZE - (lnEnd - ln));
				if ( tlen + lnEnd - ln > LN_SIZE ) {
					t[ tlen - 1 ] = '\n'; /* set the ending newline to the 't' */
					ln[LN_SIZE] = '\0'; /* terminate the string to search in */
				} else {
					lnEnd += tlen;
				}
			}

			item = hyp_node_item_next( item);
		}

		hyp_free_node(node);
	}

	return 0;
}


int main( int argc, char *argv[] )
{
	HYP *hyp = hyp_load( argv[1] );
	if ( hyp ) {
		if ( argc > 2 ) {
			int index;

			printf( "<!--refs \"prev=0&next=0&toc=0&idx=%d\"-->\n", hyp->preamble.idx_index);
			printf( "<!--title \"'"); emit_quoted( argv[2] ); printf( "' search\"-->\n");
			printf( "<!--pre-->\n");

			for (index = 0; index < hyp->header.entry_count; index++) {
				switch ( hyp->index_table[ index ].type ) {
					case HYP_IDX_NODE:
					case HYP_IDX_PNODE:
						search_node( hyp, index, argv[2] );
						break;
					default:;
				}
			}

			printf( "<!--/pre-->\n");
		}
		hyp_free( hyp );
	}
}

