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
 * $Id$
 */

#ifndef __LIBHYP__
#define __LIBHYP__


/* Version information */
#define __LIBHYP_VERSION_MAJOR__     0
#define __LIBHYP_VERSION_MINOR__     1
#define __LIBHYP_VERSION_REVISION__  0


/* .HYP file header */
typedef struct _HYP_HDOC_HEADER {
	unsigned long	magic;			/* Should be 'HDOC' */
	unsigned long	length;
	unsigned short	entry_count;
	unsigned char	hcp_version;
	unsigned char	os_id;
} HYP_HDOC_HEADER;


/***** .HYP file header index table *****/

/* HDOC_IDXITEM type constants */
#define HYP_IDX_NODE	0
#define HYP_IDX_PNODE	1
#define HYP_IDX_EXTERN	2
#define HYP_IDX_IMAGE	3
#define HYP_IDX_SYSTEM	4
#define HYP_IDX_RXS	5
#define HYP_IDX_RX	6
#define HYP_IDX_QUIT	7

typedef struct _HYP_HDOC_IDXITEM {
	unsigned char	type;
	unsigned long	offset;
	unsigned long	compressed_len;
	unsigned short	idx_next;
	unsigned short	idx_prev;
	unsigned short	idx_toc;
	char		*name;
} HYP_HDOC_IDXITEM;



/* ITEM type constants */
#define HYPT_TEXT	1
#define HYPT_EFFECTS	2
#define HYPT_LINK	3
#define HYPT_LINE	4
#define HYPT_BOX	5
#define HYPT_IMAGE	6


typedef struct _HYP_ITEM {
	/**
	 * WARNING: the next, prev members
	 * need to be in sync with the internal
	 * representation from LINKABLE from
	 * list.h to let this work
	 **/
	struct _HYP_ITEM *next;
	struct _HYP_ITEM *prev;

	short		type;
} HYP_ITEM;

typedef struct _HYP_TEXT {
	HYP_ITEM	item;
	char		*string;
} HYP_TEXT;

typedef struct _HYP_EFFECTS {
	HYP_ITEM	item;
	unsigned char	effects;
} HYP_EFFECTS;

typedef struct _HYP_LINK {
	HYP_ITEM	item;
	unsigned short	index;
	unsigned short	line;
	char		*destination;
} HYP_LINK;

typedef struct _HYP_LINE {
	HYP_ITEM	item;
	unsigned char	x_offset;
	unsigned short	y_offset;
	char		x_length;
	unsigned char	y_length;
	char		attribs;
	char		style;
} HYP_LINE;

typedef struct _HYP_BOX {
	HYP_ITEM	item;
	unsigned short	rbox_flag;	/* boolean */
	unsigned char	x_offset;
	unsigned short	y_offset;
	unsigned char	width;
	unsigned char	height;
	char		pattern;
} HYP_BOX;

typedef struct _HYP_IMAGE {
	HYP_ITEM	item;
	unsigned char	limage_flag;	/* boolean */
	unsigned char	x_offset;
	unsigned short	y_offset;
	unsigned short	dither_mask;
	unsigned short	index;
} HYP_IMAGE;


/* IMAGE data descriptor */
typedef struct _HYP_IMAGE_DATA {
	short		width		/* image width in pixels */;
	short		height;		/* image height in pixels */
	unsigned long	pitch;		/* line length in bytes */
	short		planes;		/* image color plane count (color_count = 1<<planes)*/
	void		*data;		/* plane by plane raw data */
} HYP_IMAGE_DATA;


/* NODE descriptor */
typedef struct _HYP_NODE {
	char		*name;
	char		*title;
	void		*items;
} HYP_NODE;


typedef struct _HYP {
	char              *filename;
	HYP_HDOC_HEADER   header;		/* .HYP file header */
	HYP_HDOC_IDXITEM  *index_table;		/* Index table of .HYP */

	struct {
		char	      *help;
		char	      *deflt;
		char	      *database;
		char	      *version;
		char	      *subject;
		char	      *author;
		char	      *options;
		char          *flags;		/* Supposed to cause the CHIPS60 encoding (each byte = 7F - original contents) */
		unsigned char width;		/* HYP preamble @width value */
		long          idx_index;	/* 'Index' named node index */
	} preamble;
} HYP;



HYP		*hyp_load( char *filename );
void		hyp_free( HYP *hyp);

HYP_NODE	*hyp_parse_node( HYP *Hyp, long index);
void		hyp_free_node( HYP_NODE *node);

HYP_IMAGE_DATA	*hyp_parse_image_data( HYP *hyp, unsigned short index);
void		hyp_free_image_data( HYP_IMAGE_DATA *img);

HYP_ITEM	*hyp_node_item_first( HYP_NODE *node);
HYP_ITEM	*hyp_node_item_next( HYP_ITEM *item);


#endif /* __LIBHYP__ */
