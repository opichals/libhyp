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
 * $Source: /local/libhyp.cvs/libhyp/src/list.c,v $
 *  
 * CVS info:
 *   $Author: standa $
 *   $Date: 2005-06-03 21:08:11 $
 *   $Revision: 1.1.1.1 $
 */

#include <stdlib.h>
#include "list.h"


LIST *listSplice( LIST *list, LINKABLE *first, LINKABLE *pastLast ) {
	listInit( list );

	if ( first == pastLast )
		return list;

	first->prev->next = pastLast;
	pastLast->prev->next = &list->tail;
	list->tail.prev = pastLast->prev;
	pastLast->prev = first->prev;
	first->prev = &list->head;
	list->head.next = first;
	return list;
}


LIST *createList( void ) {
	LIST *result = malloc( sizeof(LIST) );
	if ( !result )
		return NULL;

	listInit( result );
	return result;
}

