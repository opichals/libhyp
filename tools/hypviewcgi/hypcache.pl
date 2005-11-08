#!/usr/bin/perl
#
# libhyp: ST-Guide HYPertext file handling library
# Copyright (c) 2005 Standa Opichal / JAY Software
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# $Source: /local/libhyp.cvs/libhyp/tools/hypviewcgi/hypcache.pl,v $
#  
# CVS info:
#   $Author: standa $
#   $Date: 2005-11-08 00:43:24 $
#   $Revision: 1.3 $
#

sub wget_fetch {
	my( $url ) = @_;
	if ( $url eq "") { last; }

	my ( $TMP ) = "cache";

	$url =~ s/\+/ /g;  # urldecode
	$url =~ s/%([0-9a-fA-F][0-9a-fA-F])/chr(hex($1))/ge;  # urldecode

	# get the URL and strip slashes
	my $FILE = $url;
	$FILE =~ s![/\\]!_!g;

	$FILE = "$TMP/$FILE";

	if ( ! -f $FILE ) {
		#print "cached $url to $FILE";

		`wget -q -O $FILE -c $url`;
	}

	$FILE;
}

1
