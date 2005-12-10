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
# $Source: /local/libhyp.cvs/libhyp/tools/hypviewcgi/hypviewsvg.cgi,v $
#  
# CVS info:
#   $Author: standa $
#   $Date: 2005-12-10 02:13:19 $
#   $Revision: 1.1 $
#

# parse the query string
%form = map { split('=') } split('&', $ENV{QUERY_STRING});
if ( $form{svg} ) {
	$addtourl .= "&amp;svg=$form{svg}";
}

#print "Content-Type: text/html\n\n";
print "Content-Type: application/xhtml+xml\n\n";

require "./config.pl";
require "./hypcache.pl";
$form{file} = &wget_fetch( $form{url} );

$form{durl} = $form{url};
$form{durl} =~ s/\+/ /g;  # urldecode
$form{durl} =~ s/%([0-9a-fA-F][0-9a-fA-F])/chr(hex($1))/ge;  # urldecode
$form{q} =~ s/\+/ /g;  # urldecode
$form{q} =~ s/%([0-9a-fA-F][0-9a-fA-F])/chr(hex($1))/ge;  # urldecode

if ( $form{dstenc} ) {
	$ENCA = "| ./enca/bin/enca -Lcs -x \"$form{dstenc}\"";
	$addtourl .= "&amp;dstenc=$form{dstenc}";
} else {
	$form{dstenc} = "latin1";
}

if ( $form{q} ) {
	open $FH, "./grephyp \"$form{file}\" \"$form{q}\" $ENCA |";
} else  {
	open $FH, "./dehyp \"$form{file}\" \"$form{index}\" $ENCA |";
}
@lines = <$FH>;
close $FH;

if ( $#lines == -1 ) {
	unlink $form{file};
	print "Corrupted .HYP file\n";
	die(0);
}

print "<?xml version=\"1.0\" encoding=\"$form{dstenc}\" standalone=\"no\"?>\n";
print '<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd"'."\n";
print ' [<!ENTITY nbsp "&#160;">]>'."\n";

# get only the filename
($this) = ($ENV{SCRIPT_NAME} =~ m!.*/([^\/]+)!);
$offset = 0;


$oldeff = 0;
sub effects {
	my ($e) = @_;
	my ($ch) = $e ^ $oldeff;
	my ($eff) = "";

	if ($ch & 0x1 && $e & 0x1 ) {
		$eff .= "<b>";
	}
	if ($ch & 0x2 && $e & 0x2 ) {
		$eff .= "<em>";
	}
	if ($ch & 0x4 && $e & 0x4 ) {
		$eff .= "<em>";
	}
	if ($ch & 0x8 && $e & 0x8 ) {
		$eff .= "<span style=\"text-decoration: underline;\">";
	}
	if ($ch & 0x8 && !($e & 0x8) ) {
		$eff .= "</span>";
	}
	if ($ch & 0x4 && !($e & 0x4) ) {
		$eff .= "</em>";
	}
	if ($ch & 0x2 && !($e & 0x2) ) {
		$eff .= "</em>";
	}
	if ($ch & 0x1 && !($e & 0x1) ) {
		$eff .= "</b>";
	}
	$oldeff = $e;
	
	$eff."<!--ef ".$e."-->";
}

sub insertImages {
	# fixup the @(l)image ypositions positions
	foreach $imgargs (@limg_args) {
		my (%args) = %$imgargs;

		if ( $args{xoffset} == 0 ) {
			# centered image
			$images .= "\n<div align=\"center\" style=\"position:absolute; top:0em; width:78ex; z-index:$z;\">"
		} else {
			# xoffset positioned image
			$images .= "\n<div style=\"position:absolute; top:0em; left:". int($args{xoffset}*1.35) ."ex; z-index:$z;\">";
		}

		# yoffset number of newlines
		my $count = $args{yoffset};
		while ( $count-- >= -1 ) { $images .= "<br/>"; }

		# @limage additional newlines
		if ( $args{type} eq "limage" ) {
			my $count = ($args{height}+15)/16;
			my $limgnl = "";
			while ( $count-- >= -1 ) { $limgnl .= "\n"; }
			splice @lines,$begidx+$args{yoffset},0,$limgnl; $begidx += 1;
		}

		$images .= "<img src=\"hypviewimg.cgi?url=$form{url}\&amp;index=$args{index}\"/></div>";

		$z++;
	}

	$images;
}

if ( $form{ex} == 0 ) {
	$form{ex} = 8;
}
if ( $form{em} == 0 ) {
	$form{em} = 16;
}

sub constructGraphics {
	# fixup the @(l)image ypositions positions
	foreach $gr ( @graphics ) {
		my ($g);
		my (%gr) = map { split('=') } split('&', $gr);
		# adjust according to the @limage offset
		$gr{yoffset} += $offset;

		if ( $gr{cmd} eq "img" ) {
			# compute the graphics offset caused by @limage lines
			if ( $gr{type} eq "limage" ) {
				my $count = ($gr{height}+15)/16;
				$offset += $count + 1;
			}

			push @limg_args, {%gr};
		} elsif ( $gr{cmd} eq "box" ) {
			$g  = "  <svg:rect x=\"".($gr{xoffset}*$form{ex})."\" y=\"".($gr{yoffset}*$form{em})."\"";
			$g .= " width=\"".($gr{width}*$form{ex})."\" height=\"".($gr{height}*$form{em})."\" style=\"";
			if ( $gr{pattern} == 8 ) {
				$g .= "fill:black;fill-opacity:1;stroke:black;\"";
			} elsif ( $gr{pattern} != 0 ) {
				$g .= "fill:black;fill-opacity:".($gr{pattern}*0.1).";stroke:black;\"";
			} else {
				$g .= "fill:none;stroke:black;\"";
			}
			if ( $gr{rbox} != 0 ) {
				$g .= " rx=\"".(($gr{width}+$gr{height})/2)."\"";
			}
			push @graphs, "$g/>\n";
		} elsif ( $gr{cmd} eq "line" ) {
			$g  = "  <svg:line x1=\"".($gr{xoffset}*$form{ex})."\" y1=\"".($gr{yoffset}*$form{em});
			$g .= "\" x2=\"".($gr{xoffset}+$gr{xlength})*$form{ex}."\" y2=\"".($gr{yoffset}+$gr{ylength})*$form{em};
			$g .= "\" stroke=\"black\"";
			if ( $gr{style} == 2 ) {
				$g .= " stroke-dasharray=\"8,2\"";
			} elsif ( $gr{style} == 3 ) {
				$g .= " stroke-dasharray=\"2,5\"";
			} elsif ( $gr{style} == 4 ) {
				$g .= " stroke-dasharray=\"6,1,1,1\"";
			} elsif ( $gr{style} == 5 ) {
				$g .= " stroke-dasharray=\"6,6\"";
			} elsif ( $gr{style} == 6 ) {
				$g .= " stroke-dasharray=\"3,1,2,1,2,1,3,1\"";
			} elsif ( $gr{style} == 7 ) {
				$g .= " stroke-dasharray=\"1,1\"";
			}
			if ( $gr{attribs} & 1 ) {
				$g .= ' marker-start="url(#arrowbeg)"';
			}
			if ( $gr{attribs} & 2 ) {
				$g .= ' marker-end="url(#arrowend)"';
			}
			push @graphs, "$g/>\n";
		} elsif ( $gr{cmd} eq "refs" ) {
			%refs = %gr;
		}
	}
}

# collect image arguments and put them into an array
$begidx = 1;
foreach my $l ( @lines ) {
	if ( $l =~ m"<!--pre-->" ) { last; }

	$l =~ s:<!--(\S+)\s+\"([^\"]+)\"-->\n:push @graphics, "cmd=$1&$2"; "":ge;
	$begidx++;
}

&constructGraphics();

$z = 1;

my $images = "";
if ( $form{hideimages} ne "1" ) {
	$images = &insertImages();
} else {
	$addtourl .= "&amp;hideimages=$form{hideimages}";
}

$Lines = join "", @lines;

# place the text-wrapping <div> just past the last image
#$Lines =~ s!(.*)</div>!$1</div>\n<div style="position:absolute; top:0; z-index:2;">!m;

$title = "\n<title>$1 - $form{url}</title>\n" if ( $Lines =~ m|<!--title \"(.*?)\"-->| );

if ( $refs{idx} ) {
	if ( $form{hidemenu} ne "1" ) {
		$refs =  "<div style=\"position:absolute; top:0; z-index:$z;\"><form action=\"$this\" method=\"GET\"><a href=\"javascript: history.go(-1)\"><img src=\"$config{href_image}/iback.png\" border=\"0\"/></a>";
		$refs .= "\n<a href=\"$this\?url=$form{url}$addtourl&amp;index=$refs{prev}\" accesskey=\"p\" rel=\"prev\"><img src=\"$config{href_image}/iprev.png\" border=\"0\"/></a>" if ( $refs{prev} != -1 );
		$refs .= "\n<a href=\"$this\?url=$form{url}$addtourl&amp;index=$refs{toc}\" accesskey=\"t\" rel=\"contents\"><img src=\"$config{href_image}/itoc.png\" border=\"0\"/></a>" if ( $refs{toc} != -1 );
		$refs .= "\n<a href=\"$this\?url=$form{url}$addtourl&amp;index=$refs{next}\" accesskey=\"n\" rel=\"next\"><img src=\"$config{href_image}/inext.png\" border=\"0\"/></a>" if ( $refs{next} != -1 );
		$refs .= "\n<a href=\"$this\?url=$form{url}$addtourl&amp;index=$refs{idx}\" accesskey=\"z\" rel=\"index\"><img src=\"$config{href_image}/iindex.png\" border=\"0\"/></a>" if ( $refs{idx} != -1 );
		$refs .= "\n&nbsp;&nbsp;&nbsp;\n<a href=\"../libhyp\" accesskey=\"o\"><img src=\"$config{href_image}/iload.png\" border=\"0\"/></a>";
		$refs .= "\n&nbsp;&nbsp;&nbsp;\n<input type=\"hidden\" name=\"url\" value=\"$form{durl}$addtourl\"/><input style=\"position:relative; top:-6px;\" accesskey=\"s\" type=\"text\" name=\"q\" width=\"10\" value=\"$form{q}\"/></form>\n";
		$refs .= "</div>";
	} else {
		$refs = "";
		$addtourl .= "&amp;hidemenu=$form{hidemenu}";
	}

	$title .= "<link href=\"$this\?url=$form{url}$addtourl&amp;index=$refs{prev}\" accesskey=\"p\" rel=\"prev\"/>\n" if ( $refs{prev} != -1 );
	$title .= "<link href=\"$this\?url=$form{url}$addtourl&amp;index=$refs{toc}\" accesskey=\"t\" rel=\"contents\"/>\n" if ( $refs{toc} != -1 );
	$title .= "<link href=\"$this\?url=$form{url}$addtourl&amp;index=$refs{next}\" accesskey=\"n\" rel=\"next\"/>\n" if ( $refs{next} != -1 );
	$title .= "<link href=\"$this\?url=$form{url}$addtourl&amp;index=$refs{idx}\" accesskey=\"z\" rel=\"index\"/>\n" if ( $refs{idx} != -1 );
}

# HTML links (it is worth it in HTML browser ;)
$Lines =~ s|(\s)((http\|ftp):/\S+)([;:,\.\]\)\}\"\']*\s)|$1<a href="$2">$2</a>$4|gm;
$Lines =~ s|(\s)([a-z]+[a-z0-9.\-_]+\@[a-z0-9.\-_]*[a-z])([;:,\.\]\)\}\"\']*\s)|$1<a href="mailto:$2">$2</a>$3|gim;

# effects
$Lines =~ s"<!--ef 0x([0-9a-fA-F][0-9a-fA-F])-->"&effects(hex($1))"gem;

$Lines =~ s|<!--pre-->|$refs<pre>\n$images\n</pre><div style="position:absolute; top:0em; z-index:$z;"><pre>|m;
$Lines =~ s'<!--/pre-->'</pre></div>'m;

sub emitLink {
	my ($href,$text) = @_;
	$href =~ s|&line=\d+||gm; # TODO: jump to line
	$href =~ s|\&|\&amp;|gm;

	"<a href=\"$this\?url=$form{url}$addtourl&amp;$href\">$text</a>"
}

# make <a> links valid to our location
$Lines =~ s|<!--a href=\"(.*?)\"-->(.*?)<!--/a-->|emitLink($1,$2);|gem;

# move everything down (if menu is on)
$Lines =~ s|top:0em;|top:34px;|gm  if ( $form{hidemenu} ne "1" );

# strip the remaining unhandled tags and non XML characters
$Lines =~ s|<!--.*?-->||gm;
$Lines =~ s|([\x0-\x9\xb\xc\xd-\x1f])|?|gm;

#print '<html>'."\n";
print '<html xmlns="http://www.w3.org/1999/xhtml" xmlns:svg="http://www.w3.org/2000/svg">'."\n";

print '<head>'."\n";
print ' <meta http-equiv="Content-Type" content="text/html';
if ( $form{dstenc} ) {
	print '; charset='.$form{dstenc};
}
print "\"/>$title\n</head>\n";

print "<body>\n";

print '<div style="position:absolute; top:'.(30+($form{hidemenu}!=1 ? 34:0)).'px; left:0px; z-index:0;">'."\n";
print ' <svg:svg version="1.1" baseProfile="tiny">'."\n";
print "  <svg:defs>\n";
print '   <svg:marker id="arrowbeg" viewBox="0 0 10 20" refX="2" refY="10" markerUnits="strokeWidth" markerWidth="15" markerHeight="15" orient="auto">'."\n";
print '     <svg:path d="M 0 10 L 10 0 M 0 10 L 10 20" fill="black" stroke="black"/>'."\n";
print '   </svg:marker>'."\n";
print '   <svg:marker id="arrowend" viewBox="0 0 10 20" refX="8" refY="10" markerUnits="strokeWidth" markerWidth="15" markerHeight="15" orient="auto">'."\n";
print '     <svg:path d="M 10 10 L 0 0 M 10 10 L 0 20" fill="black" stroke="black"/>'."\n";
print '   </svg:marker>'."\n";
print "  </svg:defs>\n";
print @graphs;
print ' </svg:svg>'."\n";
print '</div>'."\n";

print "<div style=\"width:75ex;\">\n";
print $Lines;
# map { print "$_ -> $ENV{$_}\n"; } keys %ENV;
# map { print "$_\n"; } ( $0, $1, $2);
print "</div></body></html>";
