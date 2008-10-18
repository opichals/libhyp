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
 * $Source: /local/libhyp.cvs/libhyp/src/hypobj.h,v $
 *  
 * CVS info:
 *   $Author: standa $
 *   $Date: 2006-04-06 14:29:22 $
 *   $Revision: 1.3 $
 */


/* Extended header entry */

/* FHYPEHENTRY type constants */
#define HYP_EH_END			0
#define HYP_EH_DATABASE		1
#define HYP_EH_DEFAULT		2
#define HYP_EH_HOSTNAME		3
#define HYP_EH_OPTIONS		4
#define HYP_EH_AUTHOR		5
#define HYP_EH_VERSION		6
#define HYP_EH_HELP			7
#define HYP_EH_SUBJECT		8
#define HYP_EH_TITLETABLE	9
#define HYP_EH_STGUIDEFLAGS	10
#define HYP_EH_WIDTH		11

typedef struct {
	unsigned short	type;
	unsigned short	length;
	/*...	Data follows */
} HYP_FHYPEHENTRY;


/* default @width value constant */
#define HYP_DEFAULTWIDTH	75


/* node internal represetation tags */
#define HYP_ITITLE	35
#define HYP_ILINK	36
#define HYP_ILNLINK	37
#define HYP_IALINK	38
#define HYP_ILNALINK	39
#define HYP_IOTHER	40 /* 40 - 46 */
#define HYP_IDITHERMASK	47
#define HYP_IXREFS	48
#define HYP_ITREE	49
#define HYP_IIMAGE	50
#define HYP_ILINE	51
#define HYP_IBOX	52
#define HYP_IRBOX	53
