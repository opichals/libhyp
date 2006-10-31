#!/usr/bin/perl
#
# libhyp: ST-Guide HYPertext file handling library
# Copyright (c) 2005-2006 Standa Opichal / JAY Software
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
#   $Date: 2006-10-31 20:24:19 $
#   $Revision: 1.9 $
#

use File::Find;

sub wget_fetch {
	my( $url, $cache_path ) = @_;
	if ( $url eq "") { return; }

	my ( $TMP ) = $cache_path;

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

sub get_hyp {
	my ($url, $cache_path, $TMP, $mask) = @_;

	my $FILE = &wget_fetch( $url, $cache_path);

	if ( ! &is_hyp($FILE) ) {

		# check whether there is such .HYP file expanded already
		if ( $mask !~ m/\*\?/ && -f "$FILE#$mask" && &is_hyp("$FILE#$mask") ) {
			return ( "$FILE#$mask", $mask );
		}

		# if not configured then use /tmp as TMP
		if ( ! $TMP ) {
			$TMP = "/tmp";
		}

		# temp folder
		$TMP .= "/$$";

		# try to extract a file
		mkdir $TMP;

		if ( ! $mask ) {
			$mask = "*.hyp";
		}

		&extract( $FILE, $TMP, $mask);
		$epathname = &find_hyp($TMP, $mask);

		if ( $epathname ne "" ) {
			# move the found file $TMP/yyy/xxx to $FILE#xxx
			( $mask ) = ( $epathname =~ m/([^\/\\]+)$/ );
			$FILE .= "#$mask";
			rename $epathname, $FILE;
		}

		`rm -rf $TMP`;
	}

	( $FILE, $mask );
}


sub is_hyp {
	my ( $filename ) = @_;
	my $buffer;

	# equivalent of: if ( `head -c4 $_` eq 'HDOC' ) ...
	open $FH, $filename or return 0;
	if ( sysread($FH, $buffer, 4) == 4 && $buffer eq 'HDOC' ) {
		close $FH;
		return 1;
	}
	close $FH;

	return 0;
}


sub extract {
	my ($FILE, $dest, $mask) = @_;

	$id = `file $FILE`;
	if ( $id =~ /zip.*archive/i ) {
		$cmd = "unzip -qqjoCx -d \"$dest\" \"$FILE\" \"$mask\" 2>/dev/null";
	} elsif ( $id =~ /lha.*archive/i ) {
		$cmd = "lha -xfiw=\"$dest\" \"$FILE\" \"$mask\" 2>/dev/null";
	} elsif ( $id =~ /gzip.*compressed/i ) {
		$cmd = "tar -xz -C \"$dest\" -f \"$FILE\" \"*$mask\" 2>/dev/null";
	} elsif ( $id =~ /bzip2.*compressed/i ) {
		$cmd = "tar -xj -C \"$dest\" -f \"$FILE\" \"*$mask\" 2>/dev/null";
	}

	`$cmd`;
}


sub find_hyp {
	my ($dirname, $mask) = @_;

	# convert DOS wildcards to RE
	$mask =~ s/\./\\./g;
	$mask =~ s/\*/.\*/g;
	$mask =~ s/\?/./g;

	my @found;
	sub wanted { /^$mask$/i && push @found, "$File::Find::dir/$_"; }
	find( \&wanted, $dirname);
		print STDERR "EXT: $#found\n\n";
	foreach my $file ( @found ) {
		print STDERR "EXT: $file\n\n";

		# non-Unix archives might have no permissions
		chmod 0644, $file;

		if ( -f "$file" && &is_hyp( $file) ) {
			return $file;
		}
	}

	"";
}


1
