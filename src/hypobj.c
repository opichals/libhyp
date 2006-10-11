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
 * $Source: /local/libhyp.cvs/libhyp/src/hypobj.c,v $
 *  
 * CVS info:
 *   $Author: standa $
 *   $Date: 2006-10-11 15:16:05 $
 *   $Revision: 1.7 $
 */


#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <netinet/in.h>	/* for ntoh[sl]() */

#include <libhyp.h>

#include "list.h"
#include "hypobj.h"


#define hyp_getword( src )  ( ( ( (unsigned short)(*( ((char*)(src)) + 1) - 1 ) << 8 ) | (unsigned short)( ( *((char*)(src)) - 1 ) & 0xff ) ) - ( (*(((char*)(src)) + 1) - 1) & 0xff ) )


/* from decodlh5.c */
int decode_lh5( char *infp, char *outfp, long original_size, long packed_size );

/***** hyp_read_index_data... Loads and decompress an index entry from .HYP file *****/
char* hyp_read_index_data( HYP *hyp, unsigned long index, unsigned long *len )
{
	char* buff;
	HYP_HDOC_IDXITEM *ie = &hyp->index_table[ index ];

	FILE *fh = fopen( hyp->filename, "rb" );
	if ( fh == NULL ) {
		*len = 0;
		return NULL;
	}

	*len = hyp->index_table[ index + 1 ].offset - ie->offset;
	buff = malloc( ie->compressed_len + *len );
	if ( !buff )
		return NULL;

	fseek( fh, ie->offset, SEEK_SET );
	if ( ie->compressed_len ) {
		char* cbuff = malloc( *len );
		fread( cbuff, *len, 1, fh );
		fclose( fh );

		decode_lh5( cbuff, buff, ie->compressed_len + *len, *len );
		*len += ie->compressed_len;

		free( cbuff );
	} else {
		fread( buff, *len, 1, fh );
		fclose( fh );
	}

	/* Supposed to be the "invers byte" encoding cause */
	if ( hyp->preamble.flags != NULL ) {
		if ( *hyp->preamble.flags & 0x02 ) {
			unsigned long	tmp_length = *len;
			char*	tmpbuff = buff;

			while ( tmp_length-- ) *tmpbuff++ ^= 0x7F;
		}
	}

/***/
/*printf( "%ld", *len + ie->compressed_len );
	fh = Fls_Open( "NODE.BIN", FO_WRITE );
	Fls_Write( fh, *len, buff );
	Fls_Close( fh );
*//***/

	return buff;
}


HYP_IMAGE_DATA *hyp_parse_image_data( HYP *hyp, unsigned short index )
{
	unsigned long size;
	unsigned char* data = hyp_read_index_data( hyp, index, &size );
	if ( ! data ) return NULL;

	{
		unsigned long plane_size;
		HYP_IMAGE_DATA *img = malloc( sizeof(HYP_IMAGE_DATA));
		if ( !img ) return NULL;

		img->width	= (data[0] << 8) | data[1];
		img->height	= (data[2] << 8) | data[3];
		img->pitch	= ((img->width + 15) >> 4) << 1;
		img->planes	= data[4];
		plane_size	= img->pitch * img->height;
		img->data 	= malloc( plane_size * img->planes );
		if ( ! img->data ) {
			free(img);
			return NULL;
		}

#if 0
		printf( "img: w=%d h=%d plns=%d, pitch=%d (%d) (size=%ld)\n", img->width, img->height, img->planes, img->pitch, plane_size, size );	/***/
#endif

		/* Fill the totally filled planes */
		{
			unsigned long offset = 0;
			unsigned char plane_onoff_mask = data[6];
			while( plane_onoff_mask ) {
				if ( plane_onoff_mask & 1 ) {
					memset( img->data + offset, 0xff, plane_size );
				}
				offset += plane_size;
				plane_onoff_mask >>= 1;
			}
		}

		/* Copy the present planes data */
		{
			const char *src_data = &data[8];
			unsigned long offset = 0;
			unsigned char plane_data_mask = data[5] & ((1 << img->planes) - 1); /* limit the number of planes for the mask */
			while( plane_data_mask ) {
				if ( plane_data_mask & 1 ) {
					memcpy( img->data + offset, src_data, plane_size );
					src_data += plane_size;
				}
				offset += plane_size;
				plane_data_mask >>= 1;
			}
		}

		free(data);
		return img;
	}
}

void hyp_free_image_data( HYP_IMAGE_DATA *img )
{
	free(img->data);
	free(img);
}


static
void hyp_node_add_graphics( HYP_NODE *node, HYP_ITEM *item, short lineno ) {
	listInsert( &((LIST*)node->items)->tail, (LINKABLE*)item);
}

static
void hyp_node_add_effects( HYP_NODE *node, unsigned char e ) {
	HYP_EFFECTS *eff = malloc( sizeof(HYP_EFFECTS));
	eff->item.type = HYPT_EFFECTS;
	eff->effects = e;
	listInsert( &((LIST*)node->items)->tail, (LINKABLE*)eff);
}

static
void hyp_node_add_string( HYP_NODE *node, char *s ) {
	HYP_TEXT *txt = malloc( sizeof(HYP_TEXT));
	txt->item.type = HYPT_TEXT;
	txt->string = strdup( s );
	listInsert( &((LIST*)node->items)->tail, (LINKABLE*)txt);
}

static
void hyp_node_add_link( HYP_NODE *node, char *d, unsigned short index, unsigned short line ) {
	HYP_LINK *lnk = malloc( sizeof(HYP_LINK));
	lnk->item.type = HYPT_LINK;
	lnk->index = index;
	lnk->line = line;
	lnk->destination = strdup( d );
	listInsert( &((LIST*)node->items)->tail, (LINKABLE*)lnk);
}

/***** Hyp_DataToCache... Parses the .HYP node internal structure into the Blyp one *****/
HYP_NODE *hyp_parse_node_data( HYP *hyp, char *buff, unsigned long len )
{
	char line[256];
	HYP_NODE *node;
	char* eod = buff + len; /* end of data */
	char* dest = line;
	char comm;
	unsigned short line_number, prev_line_number = -1;

	node = malloc( sizeof(HYP_NODE) );
	if (!node) return NULL;

	node->name = NULL;
	node->title = NULL;
	node->items = createList();

	while ( buff < eod ) {
		*dest = *buff++;

		if ( *dest == '\0' ) { /* EOL */
			*dest++ = '\n'; /* add newline character */
		       	*dest = '\0'; dest = line;
			hyp_node_add_string( node, line );
		} else if ( *dest != 033 ) { /* not ESC -> common char */
			dest++;
		} else switch ( comm = *buff++ ) {
			case 033: /* ESC character */
				dest++;
				break;

			case HYP_IIMAGE: /* Len = 7 */
			{
				HYP_IMAGE *img = malloc( sizeof(HYP_IMAGE) );
				img->item.type = HYPT_IMAGE;

				img->index = hyp_getword( buff ); buff += 2;	/* 2 */
				img->x_offset = *buff++;			/* 1 */
				line_number = hyp_getword( buff ); buff += 2;	/* 2 */
				img->y_offset = line_number;
				img->limage_flag = ( *buff && prev_line_number != line_number );
				prev_line_number = line_number;

				buff += 2;	/* 2 - officially width and height ignored */
				hyp_node_add_graphics( node, (HYP_ITEM*)img, line_number );
				break;
			}

			case HYP_ILINE: /* Len = 6 */
			{
				HYP_LINE *ln = malloc( sizeof(HYP_LINE) );
				ln->item.type = HYPT_LINE;
				ln->x_offset	= *buff++;
				line_number = hyp_getword( buff ); buff += 2;
				ln->y_offset = line_number;
				ln->x_length	= *buff++ - 0x80;
				ln->y_length	= *buff++ - 1;
				ln->attribs	= (*buff - 1) & 3;
				ln->style	= ((*buff - 1) >> 3) + 1; buff++;
				hyp_node_add_graphics( node, (HYP_ITEM*)ln, line_number );

				break;
			}
			case HYP_IBOX:
			case HYP_IRBOX: /* Len = 6 */
			{
				HYP_BOX *box = malloc( sizeof(HYP_BOX) );
				box->item.type = HYPT_BOX;
				box->rbox_flag = (comm == HYP_IRBOX);
				box->x_offset	= *buff++;
				line_number = hyp_getword( buff ); buff += 2;
				box->y_offset = line_number;
				box->width	= *buff++;
				box->height	= *buff++;
				box->pattern	= *buff++ - 1;
				hyp_node_add_graphics( node, (HYP_ITEM*)box, line_number );

				break;
			}
			case HYP_IXREFS:
				buff += *buff - 2;
				/* FIXME: TODO */
				break;
			case HYP_IOTHER:
				buff += *buff - 2;
				/* FIXME: TODO */
				break;
			case HYP_IDITHERMASK:
				/* FIXME: TODO: Convert the Images dither mask */
				buff += *buff - 2;
				break;
			case HYP_ITITLE:
				node->title = strdup( buff );
				buff += strlen( buff ) + 1;
				break;
			case HYP_ITREE:
				/* FIXME: TODO: RSC trees */
				buff += 8;
				break;
			case HYP_ILNLINK:
			case HYP_ILNALINK:
			case HYP_ILINK:
			case HYP_IALINK:
			{
				HYP_HDOC_IDXITEM *ie;
				unsigned short index;
				unsigned short lineno;
				int len;

				if ( ( comm == HYP_ILNLINK ) || ( comm == HYP_ILNALINK ) ) {
					lineno = hyp_getword( buff ); buff += 2; /* 2 */
				} else {
					lineno = 0;
				}

				index = hyp_getword( buff ); buff += 2;		/* 2 */
				ie = &hyp->index_table[ index ];
				len = *buff++ - 32;				/* 1 */

				*dest = '\0';
				if ( dest != line) {
					dest = line;
					hyp_node_add_string( node, line );
				}

				/* To have the 0 ending copy to search for */
				if ( len ) {
					strncpy( dest, buff, len );
					buff += len;
			        } else {
					strcpy(  dest, ie->name );
					len = strlen( dest);
				}
				dest += len;

				*dest = '\0'; dest = line;
				hyp_node_add_link( node, line, index, lineno );
				break;
			}

			default:
				if ( comm >= 100 ) {
					unsigned char effects = comm - 100;
					*dest = '\0';
					if ( dest != line) {
						dest = line;
						hyp_node_add_string( node, line );
					}
					hyp_node_add_effects( node, effects );
				}
				break;
		}
	}

	return node;
}


static
void hyp_free_node_item( HYP_ITEM *item )
{
	switch ( item->type ) {
		case HYPT_TEXT:
			free( ((HYP_TEXT*)item)->string);
			break;
		case HYPT_LINK:
			free( ((HYP_LINK*)item)->destination);
			break;
	}
	free( item);
}

void hyp_free_node( HYP_NODE *node )
{
	HYP_ITEM *next, *trash = (HYP_ITEM*)listFirst( (LIST*)node->items );
	while ( trash ) {
		next = (HYP_ITEM*)listNext( trash);
		hyp_free_node_item( trash );
		trash = next;
	}
	free(node->items);
	free(node->title);
	free(node);
}

HYP_ITEM *hyp_node_item_first( HYP_NODE *node)
{
	return (HYP_ITEM*)listFirst( (LIST*)node->items );
}

HYP_ITEM *hyp_node_item_next( HYP_ITEM *item)
{
	return (HYP_ITEM*)listNext( item);
}


/***** hyp_parse_node... Loads and decompress a node from .HYP file *****/
HYP_NODE *hyp_parse_node( HYP *hyp, long index )
{
	unsigned long len;
	char* buffer = hyp_read_index_data( hyp, index, &len );
	if ( buffer) {
		HYP_NODE *node = hyp_parse_node_data( hyp, buffer, len );
		node->name = hyp->index_table[ index ].name;
		free( buffer);
		return node;
	}

	return NULL;
}

static
void hyp_parse_ext_header( HYP *hyp, HYP_FHYPEHENTRY *header_ext, char *buff )
{
	switch ( header_ext->type ) {
		case HYP_EH_DATABASE:
			hyp->preamble.database	= strdup( buff );
			break;
		case HYP_EH_DEFAULT:
			hyp->preamble.deflt	= strdup( buff );
			break;
		case HYP_EH_HOSTNAME:
			/* FIXME: TODO: @hostnames */
			break;
		case HYP_EH_OPTIONS:
			hyp->preamble.options	= strdup( buff );
			break;
		case HYP_EH_AUTHOR:
			hyp->preamble.author	= strdup( buff );
			break;
		case HYP_EH_VERSION: 
			hyp->preamble.version	= strdup( buff );
			break;
		case HYP_EH_HELP:
			hyp->preamble.help	= strdup( buff );
			break;
		case HYP_EH_SUBJECT:
			hyp->preamble.subject	= strdup( buff );
			break;
		case HYP_EH_TITLETABLE:
			/* FIXME: TODO: ??? */
			break;
		case HYP_EH_STGUIDEFLAGS:
			hyp->preamble.flags	= strdup( buff );
			break;
		case HYP_EH_WIDTH:
			hyp->preamble.width	= (unsigned char)*buff;
			break;
	}
}

/***** Hyp_Load... Loads the .HYP file into the HYP *****/
HYP* hyp_load( char *filename )
{
	FILE *fh;
	HYP_FHYPEHENTRY	header_ext;
	unsigned long entry_offset;
	unsigned short index;

	HYP *hyp = malloc( sizeof(HYP) );
	if ( ! hyp )
		return NULL;

	hyp->filename = strdup(filename);

	fh = fopen( filename, "rb" );
	if ( fh == NULL ) {
		free(hyp);
		return NULL;
	}

	fread( &hyp->header, sizeof( HYP_HDOC_HEADER ), 1, fh );
	if ( ntohl(hyp->header.magic) != 0x48444f43UL /*'HDOC'*/ ) return NULL;

	hyp->preamble.database = NULL;
	hyp->preamble.deflt = NULL;
	hyp->preamble.options = NULL;
	hyp->preamble.author = NULL;
	hyp->preamble.version = NULL;
	hyp->preamble.help = NULL;
	hyp->preamble.subject = NULL;
	hyp->preamble.flags = NULL;
	hyp->preamble.width = HYP_DEFAULTWIDTH;

	/* fixup the values to CPU endian */
	hyp->header.length = ntohl(hyp->header.length);
	hyp->header.entry_count = ntohs(hyp->header.entry_count);

	/* index table load */
	entry_offset = ( hyp->header.entry_count + 1 ) * sizeof(HYP_HDOC_IDXITEM);

	if ( ! ( hyp->index_table = malloc( entry_offset + hyp->header.length ) ) ) return NULL;
	entry_offset += (long)hyp->index_table;

	fread( (void*)entry_offset, hyp->header.length, 1, fh );

	/* extended header scan */
	for(;;) {
		fread( &header_ext, sizeof( HYP_FHYPEHENTRY ), 1, fh);
		header_ext.type = ntohs(header_ext.type);
		if ( header_ext.type == HYP_EH_END )
			break;

		header_ext.length = ntohs(header_ext.length);
		if (header_ext.length <= 256) {
			char buff[256];

			fread( buff, header_ext.length, 1, fh);
			hyp_parse_ext_header( hyp, &header_ext, buff );
		} else {
			char *buff = malloc(header_ext.length);

			fread( buff, header_ext.length, 1, fh);
			hyp_parse_ext_header( hyp, &header_ext, buff );
			free(buff);
		}
	}

	// past the last trailing entry
	{
		HYP_HDOC_IDXITEM *ie = &hyp->index_table[ hyp->header.entry_count ];
		ie->type = 0;
		// set the offset to the file size
		fseek( fh, 0, SEEK_END);
		ie->offset = ftell(fh);
		ie->compressed_len = 0;
		ie->idx_next = 1;
		ie->idx_prev = 1;
		ie->idx_toc = 1;
		ie->name = "";
	}

	fclose( fh );

	/* index table scan */
	hyp->preamble.idx_index = -1;

	for( index = 0; index < hyp->header.entry_count; index++ ) {
		HYP_HDOC_IDXITEM *ie = &hyp->index_table[ index ];
		unsigned char *o = (unsigned char*)entry_offset;

		ie->type = o[1];
		ie->offset = ntohl(*(unsigned long*)&o[2]);
		ie->compressed_len = ntohs(*(unsigned short*)&o[6]);
		ie->idx_next = ntohs(*(unsigned short*)&o[8]);
		ie->idx_prev = ntohs(*(unsigned short*)&o[10]);
		ie->idx_toc = ntohs(*(unsigned short*)&o[12]);
		ie->name = (char*)&o[14];

		/* image node has the compression difference encoded as long (e.g. atos9701.hyp 274) */
		if ( ie->type == HYP_IDX_IMAGE ) {
			ie->compressed_len |= (unsigned long)ie->idx_next << 16;
		}

		/* Search for the "index" named node (if any) */
		if ( hyp->preamble.idx_index == -1 &&
				ie->type == HYP_IDX_NODE &&
				!strcmp( "Index", ie->name ) )
			hyp->preamble.idx_index = index;

		entry_offset += o[0];
	}

	return hyp;
}

static
void hyp_free_preamble( HYP *hyp )
{
	free(hyp->preamble.database);
	free(hyp->preamble.deflt);
	free(hyp->preamble.options);
	free(hyp->preamble.author);
	free(hyp->preamble.version);
	free(hyp->preamble.help);
	free(hyp->preamble.subject);
	free(hyp->preamble.flags);
}

void hyp_free( HYP *hyp )
{
	hyp_free_preamble(hyp);

	free(hyp->index_table);
	free(hyp->filename);
	free(hyp);
}
