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

#include <stdlib.h>
#include <libpng12/png.h>

#include <libhyp.h>

/* inline the palette definition */
#include "palette.c"

char emit_image_png(FILE *fp, HYP_IMAGE_DATA *img)
{
	png_structp png_ptr;
	png_infop info_ptr;

	/* Allocate basic libpng structures */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (!png_ptr) { return 1; }

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_write_struct(&png_ptr, 0);
		return 1;
	}

	/* setjmp() must be called in every function
	 * that calls a PNG-reading libpng function */
	if (setjmp(png_ptr->jmpbuf))
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return 3;
	}

	png_init_io(png_ptr, fp);

	/* set the zlib compression level */
	png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

	/* write PNG info to structure */
	png_set_IHDR(png_ptr, info_ptr, img->width, img->height, img->planes < 8 ? 8 : img->planes,
			PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	{
		png_uint_32 pal_cols = sizeof(palette)/sizeof(*palette);
		/* adjust the plane count to 8 and up */
		pal_cols = pal_cols > 1 << img->planes ? 1 << img->planes : pal_cols;
		/* The last color in GEM palette is always the 256th one regardless to the number of planes */
		if ( pal_cols < 256 ) palette[ pal_cols-1 ] = palette[ 255 ];

		png_set_PLTE(png_ptr, info_ptr, palette, pal_cols );
	}

	png_write_info(png_ptr, info_ptr);

	/* setjmp() must be called in every function
	 * that calls a PNG-writing libpng function */
	if (setjmp(png_ptr->jmpbuf))
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return 3;
	}

	{
		/* convert the raw plane by plane data to indexed chunky row by row */
		png_uint_32 l, x;
		png_uint_32 plane_size = (png_uint_32)img->pitch * img->height;
		png_bytep rowdata = malloc( img->width);
		for (l = 0; l < img->height; ++l) {
			for (x = 0; x < img->width; x++) {
				png_uint_32 p;
				unsigned char *data = img->data + (l * (png_uint_32)img->pitch) + (x / 8);
				unsigned char bitmask = 1 << (7 - (x % 8));
				unsigned char color = 0;
				for (p = 0; p < img->planes; ++p ) {
					color |= ( data[p * plane_size] & bitmask ) ? (1 << p) : 0;
				}
				rowdata[x] = color;
			}
			png_write_row(png_ptr, rowdata);
		}
		free( rowdata);
	}

	png_write_end(png_ptr, 0);

	png_destroy_write_struct(&png_ptr, &info_ptr);
	return 0;
}

