#!/usr/bin/perl
#
# libhyp: ST-Guide HYPertext file handling library
# Copyright (c) 2005-2008 Standa Opichal / JAY Software
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
# $Id$
#

use URI::Escape;

# parse the query string
%form = map { split('=') } split('&', $ENV{QUERY_STRING});

print "Content-Type: image/png\n\n";

require "./config.pl";
require "./hypcache.pl";
( $form{file}, $mask ) = &get_hyp( uri_unescape( $form{url}), $config{cache}, $config{tmp}, $form{mask} );

system("./dehyp $form{file} $form{index}");
