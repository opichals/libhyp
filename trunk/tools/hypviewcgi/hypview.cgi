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
# $Source: /local/libhyp.cvs/libhyp/tools/hypviewcgi/hypview.cgi,v $
#  
# CVS info:
#   $Author: standa $
#   $Date: 2005-07-12 04:53:08 $
#   $Revision: 1.4 $
#

# parse the query string
%form = map { split('=') } split('&', $ENV{QUERY_STRING});

print "Content-Type: text/html\n\n";

require "./config.pl";
require "./hypcache.pl";
$form{file} = &wget_fetch( $form{url} );

if ( $form{dstenc} ) {
	$ENCA = "| ./enca/bin/enca -Lcs -x $form{dstenc}";
	$addtourl .= "&dstenc=$form{dstenc}";
}

open $FH, "./dehyp $form{file} $form{index} $ENCA |";
@lines = <$FH>;
close $FH;

if ( $#lines == -1 ) {
	unlink $form{file};
	print "Corrupted .HYP file\n";
	die(0);
}

# get only the filename
($this) = ($0 =~ m!.*/([^\/]+)!);

$oldeff = 0;
sub effects {
	my ($e) = @_;
	my ($ch) = $e ^ $oldeff;
	my ($eff) = "";

	if ($ch & 0x1) {
		if ( $e & 0x1 ) {
			$eff .= "<b>";
		} else {
			$eff .= "</b>";
		}
	}
	if ($ch & 0x2) {
		if ( $e & 0x2 ) {
			$eff .= "<em>";
		} else {
			$eff .= "</em>";
		}
	}
	if ($ch & 0x4) {
		if ( $e & 0x4 ) {
			$eff .= "<em>";
		} else {
			$eff .= "</em>";
		}
	}
	if ($ch & 0x8) {
		if ( $e & 0x8 ) {
			$eff .= "<span style=\"{text-decoration: underline}\">";
		} else {
			$eff .= "</span>";
		}
	}
	$oldeff = $e;
	
	$eff."<!--ef ".$e."-->";
}

sub insertImages {
	# fixup the @(l)image ypositions positions
	while ( $imgargs = shift @limg_args ) {
		my (%args) = map { split('=') } split('&', $imgargs);

		if ( $args{xoffset} == 0 ) {
			# centered image
			$images .= "\n<div align=\"center\" style=\"position:absolute; top:0em; width:78ex; z-index:$z;\">"
		} else {
			# xoffset positioned image
			$images .= "\n<div style=\"position:absolute; top:0em; left:". ($args{xoffset}) ."ex; z-index:$z;\">";
		}

		# yoffset number of newlines
		my $count = $args{yoffset} + $offset;
		while ( $count-- > 0 ) { $images .= "\n"; }

		# @limage additional newlines
		if ( $args{type} eq "limage" ) {
			my $count = ($args{height}+15)/16;
			$offset += $count + 1;
			my $limgnl = "";
			while ( $count-- > 0 ) { $limgnl .= "\n"; }
			splice @lines,$begidx+$args{yoffset},0,$limgnl; $begidx += 1;
		}
		#print "$imgargs<br>\n";

		$images .= "<img src=\"hypviewimg.cgi?url=$form{url}\&$imgargs\"></div>";

		$z++;
	}

	$images;
}


# collect image arguments and put them into an array
$begidx = 1;
foreach my $l ( @lines ) {
	if ( $l =~ m"<!--pre-->" ) { last; }

	# strip image tags and put them into limg_args
	if ( $l =~ s"<!--img src=\"([^\"]+)\"-->""g ) {
		push @limg_args, $1;
	}
	$begidx++;
}


$z = 1;

my $images = "";
if ( $form{hideimages} ne "1" ) {
	$images = &insertImages();
} else {
	$addtourl .= "&hideimages=$form{hideimages}";
}

$Lines = join "", @lines;

# place the text-wrapping <div> just past the last image
#$Lines =~ s!(.*)</div>!$1</div>\n<div style="position:absolute; top:0; z-index:2;">!m;

$title = "\n<title>$1 - $form{url}</title>\n" if ( $Lines =~ m|<!--title \"(.*?)\"-->| );

$refs = $1 if ( $Lines =~ m|<!--refs \"(.*?)\"-->| );
if ( $refs ne "" ) {
	%refs = map { split('=') } split('&', $refs);
	if ( $form{hidemenu} ne "1" ) {
		$refs =  '<div style="position:absolute; top:0; z-index:$z;"><a href="javascript: history.go(-1)">'."<img src=\"$config{href_image}/iback.png\" border=0></a>";
		$refs .= "\n<a href=\"$this\?url=$form{url}$addtourl&index=$refs{prev}\" accesskey=\"p\" rel=\"prev\"><img src=\"$config{href_image}/iprev.png\" border=0></a>" if ( $refs{prev} != -1 );
		$refs .= "\n<a href=\"$this\?url=$form{url}$addtourl&index=$refs{toc}\" accesskey=\"t\" rel=\"contents\"><img src=\"$config{href_image}/itoc.png\" border=0></a>" if ( $refs{toc} != -1 );
		$refs .= "\n<a href=\"$this\?url=$form{url}$addtourl&index=$refs{next}\" accesskey=\"n\" rel=\"next\"><img src=\"$config{href_image}/inext.png\" border=0></a>\n" if ( $refs{next} != -1 );
		$refs .= "\n<a href=\"$this\?url=$form{url}$addtourl&index=$refs{idx}\" accesskey=\"z\" rel=\"index\"><img src=\"$config{href_image}/iindex.png\" border=0></a>\n" if ( $refs{idx} != -1 );
		$refs .= "\n<span>&nbsp;&nbsp;&nbsp;</span><a href=\"$config{href_openhyp}\" accesskey=\"o\"><img src=\"$config{href_image}/iload.png\" border=0></a>\n";
		$refs .= "</div>";
	} else {
		$refs = "";
		$addtourl .= "&hidemenu=$form{hidemenu}";
	}

	$title .= "<link href=\"$this\?url=$form{url}$addtourl&index=$refs{prev}\" accesskey=\"p\" rel=\"prev\">\n" if ( $refs{prev} != -1 );
	$title .= "<link href=\"$this\?url=$form{url}$addtourl&index=$refs{toc}\" accesskey=\"t\" rel=\"contents\">\n" if ( $refs{toc} != -1 );
	$title .= "<link href=\"$this\?url=$form{url}$addtourl&index=$refs{next}\" accesskey=\"n\" rel=\"next\">\n" if ( $refs{next} != -1 );
	$title .= "<link href=\"$this\?url=$form{url}$addtourl&index=$refs{idx}\" accesskey=\"z\" rel=\"index\">\n" if ( $refs{idx} != -1 );
}

# HTML links (it is worth it in HTML browser ;)
$Lines =~ s|(\s)((http\|ftp):/\S+)([;:,\.\]\)\}\"\']*\s)|$1<a href="$2">$2</a>$4|gm;
$Lines =~ s|(\s)([a-z]+[a-z0-9.\-_]+\@[a-z0-9.\-_]*[a-z])([;:,\.\]\)\}\"\']*\s)|$1<a href="mailto:$2">$2</a>$3|gim;

# effects
$Lines =~ s"<!--ef 0x([0-9a-fA-F][0-9a-fA-F])-->"&effects(hex($1))"gem;

$Lines =~ s|<!--pre-->|$refs<pre>\n$images\n</pre><div style="position:absolute; top:0em; z-index:$z;"><pre>|m;
$Lines =~ s'<!--/pre-->'</pre></div>'m;

# make <a> links valid to our location
$Lines =~ s|<!--a href=\"(.*?)\"-->(.*?)<!--/a-->|<a href=\"$this\?url=$form{url}$addtourl&$1\">$2</a>|gm;

# move everything down (if menu is on)
$Lines =~ s|top:0em;|top:34px;|gm  if ( $refs ne "" );

# strip the remaining unhandled tags
$Lines =~ s|<!--.*?-->||gm;

print '<HTML><HEAD><META HTTP-EQUIV="Content-Type" CONTENT="text/html';
if ( $form{dstenc} ) {
	print '; charset='.$form{dstenc};
}
print "\">$title</HEAD>";
print "<BODY><div style=\"width:75ex;\">\n";
print $Lines;
print "</div></BODY></HTML>";

