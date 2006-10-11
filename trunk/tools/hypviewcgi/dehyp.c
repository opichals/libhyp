/*
 * libhyp: ST-Guide HYPertext file handling library
 * Copyright (c) 2005-2006 Standa Opichal / JAY Software
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
 * $Source: /local/libhyp.cvs/libhyp/tools/hypviewcgi/dehyp.c,v $
 *  
 * CVS info:
 *   $Author: standa $
 *   $Date: 2006-10-11 15:16:17 $
 *   $Revision: 1.9 $
 */

#include <stdio.h>
#include <sys/fcntl.h>

#include <libhyp.h>

/* from emitpng.c */
char emit_image_png(FILE *fp, HYP_IMAGE_DATA *img);


int emit_image( HYP *hyp, int index )
{
	HYP_IMAGE_DATA *img = hyp_parse_image_data( hyp, index );
	if ( img ) {
#if 0
		FILE *fp = fopen("xxx.png", "wb");
#else
		/* stdout in binary mode */
		FILE *fp = fdopen(fileno(stdout), "wb");
#endif
		int res = emit_image_png( fp, img);
		fclose(fp);

		hyp_free_image_data(img);
		hyp_free(hyp);
		return res;
	}
	return 10;
}


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

int emit_node( HYP *hyp, int index )
{
	HYP_NODE *node = hyp_parse_node( hyp, index);
	if ( node ) {
		int first = 1;
		HYP_HDOC_IDXITEM *ie = &hyp->index_table[ index ];
		HYP_ITEM *item;

		printf( "<!--title \"");
		emit_quoted( node->title ? node->title : node->name );
		printf( "\"-->\n");

		printf( "<!--refs \"prev=%d&next=%d&toc=%d&idx=%d\"-->\n", ie->idx_prev, ie->idx_next, ie->idx_toc, hyp->preamble.idx_index);

		item = hyp_node_item_first( node );
		while ( item ) {
			switch ( item->type ) {
				case HYPT_TEXT:
					if ( first ) { printf( "<!--pre-->\n"); first=0; }
					emit_quoted( ((HYP_TEXT*)item)->string );
					break;
				case HYPT_LINK:
					if ( first ) { printf( "<!--pre-->\n"); first=0; }
					if ( hyp->index_table[ ((HYP_LINK*)item)->index ].type == HYP_IDX_EXTERN ) {
						HYP_NODE *extnode = hyp_parse_node( hyp, ((HYP_LINK*)item)->index);
						if ( !extnode ) {
							emit_quoted( ((HYP_LINK*)item)->destination );
							break;
						}
						printf( "<!--a href=\"extern=%s\"-->",extnode->name);
						hyp_free_node(extnode);
					} else {
						printf( "<!--a href=\"index=%d&line=%d\"-->", ((HYP_LINK*)item)->index, ((HYP_LINK*)item)->line+1);
					}
					emit_quoted( ((HYP_LINK*)item)->destination );
					printf( "<!--/a-->");
					break;
				case HYPT_EFFECTS:
					if ( first ) { printf( "<!--pre-->\n"); first=0; }
					printf( "<!--ef 0x%02x-->", (int)((HYP_EFFECTS*)item)->effects );
					break;
				case HYPT_LINE:
					printf( "<!--line \"xoffset=%d&yoffset=%d&xlength=%d&ylength=%d&attribs=%d&style=%d\"-->\n",
							(int)((HYP_LINE*)item)->x_offset,
							(int)((HYP_LINE*)item)->y_offset,
							(int)((HYP_LINE*)item)->x_length,
							(int)((HYP_LINE*)item)->y_length,
							(int)((HYP_LINE*)item)->attribs,
							(int)((HYP_LINE*)item)->style);
					break;
				case HYPT_BOX:
					printf( "<!--box \"rbox=%d&xoffset=%d&yoffset=%d&width=%d&height=%d&pattern=%d\"-->\n",
							(int)((HYP_BOX*)item)->rbox_flag?1:0,
							(int)((HYP_BOX*)item)->x_offset,
							(int)((HYP_BOX*)item)->y_offset,
							(int)((HYP_BOX*)item)->width,
							(int)((HYP_BOX*)item)->height,
							(int)((HYP_BOX*)item)->pattern);
					break;
				case HYPT_IMAGE:
					{
						HYP_IMAGE_DATA *img = hyp_parse_image_data( hyp, ((HYP_IMAGE*)item)->index );
						if ( img ) {
							printf( "<!--img \"index=%d&xoffset=%d&yoffset=%d&type=%simage&width=%d&height=%d\"-->\n",
									(int)((HYP_IMAGE*)item)->index,
									(int)((HYP_IMAGE*)item)->x_offset,
									(int)((HYP_IMAGE*)item)->y_offset,
									((HYP_IMAGE*)item)->limage_flag ? "l" : "",
									img->width,
									img->height );
							hyp_free_image_data(img);
						}
					}
					break;
			}
			item = hyp_node_item_next( item);
		}

		hyp_free_node(node);

		if ( !first) printf( "<!--/pre-->\n");
	}

	return 0;
}


int main( int argc, char *argv[] )
{
	HYP *hyp = hyp_load( argv[1] );
	if ( hyp ) {
		int index = argc > 2 ? atol( argv[2] ) : 0;
		int res;

		switch ( hyp->index_table[ index ].type ) {
			case HYP_IDX_IMAGE :
				res = emit_image( hyp, index );
				break;
			default :
				res = emit_node( hyp, index );
		}
		hyp_free( hyp );
		return res;
	}
	return 1;
}
