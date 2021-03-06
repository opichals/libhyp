/*
 * libhyp: ST-Guide HYPertext file handling library
 * Copyright (c) 2005-2019 Standa Opichal / JAY Software
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

#include <libhyp.h>

/* from grep.c */
void grep( HYP *hyp, const char *q );

int main( int argc, char *argv[] )
{
	/* quit with error if no arguments */
	if (argc < 2)
		return 1;

	HYP *hyp = hyp_load( argv[1] );
	if ( hyp ) {
		if ( argc > 2 ) {
            grep( hyp, argv[2] );
		}
		hyp_free( hyp );
	}
}

