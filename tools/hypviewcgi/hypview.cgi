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

my %au;

# make the svg format the default output
$dosvg = $form{svg};
if ( $dosvg eq "" ) {
	$dosvg = 1;
} else {
	$au{svg} = $form{svg};
}

$form{html} = 0;
if ( $ENV{HTTP_ACCEPT} =~ m!application/xhtml\+xml! ) {
	$form{ContentType} = "application/xhtml+xml";
} else {
	$form{ContentType} = "text/html";
	$form{html} = 1;
}
print "Content-Type: $form{ContentType}\n\n";

require "./config.pl";
require "./hypcache.pl";

$form{durl} = uri_unescape( $form{url});
$form{q} = uri_unescape( $form{q});
$form{node} = uri_unescape( $form{node});

( $form{file}, $au{mask} ) = &get_hyp( $form{durl}, $config{cache}, $config{tmp}, $form{mask} );


if ( $form{dstenc} && $config{enca} ) {
	$ENCA = "| $config{enca} \"$form{dstenc}\"";
	$au{dstenc} = $form{dstenc};
} else {
	$ENCA = "| ./st2latin1.pl";
	$form{dstenc} = "latin1";
}

if ( $form{q} ) {
	open $FH, "./grephyp \"$form{file}\" \"$form{q}\" $ENCA |";
} elsif ( $form{node} )  {
	open $FH, "./dehyp \"$form{file}\" \"node:$form{node}\" $ENCA |";
} else  {
	open $FH, "./dehyp \"$form{file}\" \"$form{index}\" $ENCA |";
}
@lines = <$FH>;
close $FH;

if ( ! $form{html} ) {
	print "<?xml version=\"1.0\" encoding=\"$form{dstenc}\"?>\n";
	print '<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd"'."\n";
	print ' [<!ENTITY nbsp "&#160;">]>'."\n";
}

if ( $#lines == -1 ) {
	unlink $form{file};
	print '<html';
	print ' xmlns="http://www.w3.org/1999/xhtml"' if ( ! $form{html} );
	print ">\n";
	print "<body>Corrupted .HYP file<br/><br/>Fix the 'url=$form{url}' to point to a real ST-Guide .HYP file.</body>\n</html>\n";
	die(0);
}


# get only the filename
($this) = ($ENV{SCRIPT_NAME} =~ m!.*/([^\/]+)!);
%max = {};
$offset = 0;


$oldeff = 0;
sub effects {
	my ($e) = @_;
	my ($ch) = $e ^ $oldeff;
	my ($ne) = ~$e;
	my ($eff) = "";
	
	# {i}ggg{u}xxx{b}yyy{U}zx{I}zz{B}
	# no optimizations for now here simple close all $oldeff and open the $e
	$ne = $oldeff;
	$ch = $oldeff;
	$ekeep = $e;
	#

	if ($ch & 0x8 && $ne & 0x8 ) {
		$eff .= "</span>";
	}
	if ($ch & 0x4 && $ne & 0x4 ) {
		$eff .= "</em>";
	}
	if ($ch & 0x2 && $ne & 0x2 ) {
		$eff .= "</em>";
	}
	if ($ch & 0x1 && $ne & 0x1 ) {
		$eff .= "</b>";
	}

	$ch |= $ekeep;
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
	$oldeff = $e;
	
	$eff."<!--ef ".$e."-->";
}

sub insertImages {
	# fixup the @(l)image ypositions positions
	foreach $imgargs (@limg_args) {
		my (%args) = %$imgargs;

		if ( $args{xoffset} == 0 ) {
			# centered image
			$images .= "<div class=\"imgCenter\" style=\"z-index:$z;\"><div><pre>";
		} else {
			# xoffset positioned image
			$images .= "<div class=\"imgDiv\" style=\"left:". int($args{xoffset}*1.35) ."ex; z-index:$z;\"><div><pre>";
		}

		# yoffset number of newlines
		my $count = $args{yoffset};
		while ( $count-- > 0 ) { $images .= "<br/>"; }

		# @limage additional newlines
		if ( $args{type} eq "limage" ) {
			my $count = (($args{height}+15)/16) - 1;
			my $limgnl = "";
			while ( $count-- >= -1 ) { $limgnl .= "\n"; }
			splice @lines,$begidx+$args{ytextoffset},0,$limgnl; $begidx += 1;
		}

		$images .= "<img src=\"hypviewimg.cgi?url=$form{url}".($au{mask} ne "" ? "\&amp;mask=$au{mask}" : "")."\&amp;index=$args{index}\"/></pre></div></div>\n";

		$z++;
	}

	$images;
}

# the following needs to be somehow calibrated (the values are
# just empirical from OS X browser versions (default font settings)
if ( $form{ex} == 0 ) {
	$form{ex} = 1; # Safari
	$form{ex} = 1.077; # Firefox
}
if ( $form{em} == 0 ) {
	$form{em} = 0.9376; $svgpos = -2;  # Safari
	$form{em} = 0.8124; $svgpos = -4; # Opera
	$form{em} = 0.875; $svgpos = -4; # Firefox 3.x
}
# $form{ex} = 1; $form{em} = 1; $svgpos = 0;


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
				$gr{ytextoffset} += $gr{yoffset} - $offset;

				my $count = ($gr{height}+15)/16;
				$offset += $count + 1;
			}

			push @limg_args, {%gr};
		} elsif ( $gr{cmd} eq "box" ) {
			$g  = "  <svg:rect x=\"".($gr{xoffset}*$form{ex})."ex\" y=\"".($gr{yoffset}*$form{em}+1)."em\"";
			$g .= " width=\"".($gr{width}*$form{ex})."ex\" height=\"".($gr{height}*$form{em})."em\" style=\"";
			if ( $gr{pattern} == 8 ) {
				$g .= "fill:rgb(0,0,0);\"";
			} elsif ( $gr{pattern} != 0 ) {
				my($grey) = (8-$gr{pattern})*32;
				$g .= "fill:rgb($grey,$grey,$grey);\"";
			} else {
				$g .= "fill:none;\"";
			}
			if ( $gr{rbox} != 0 ) {
				$g .= " rx=\"".(($gr{width}+$gr{height})/2)."\"";
			}
			push @graphs, "$g/>\n";

			$gr{width} = $gr{xoffset}+$gr{width};
			$gr{height} = $gr{yoffset}+$gr{height};
			$max{width} = $max{width}>$gr{width}?$max{width}:$gr{width};
			$max{height} = $max{height}>$gr{height}?$max{height}:$gr{height};
		} elsif ( $gr{cmd} eq "line" ) {
			$g  = "  <svg:line x1=\"".($gr{xoffset}*$form{ex})."ex\" y1=\"".($gr{yoffset}*$form{em}+1);
			$g .= "em\" x2=\"".($gr{xoffset}+$gr{xlength})*$form{ex}."ex\" y2=\"".(($gr{yoffset}+$gr{ylength})*$form{em}+1)."em\"";
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

			$gr{width} = $gr{xoffset} + ($gr{xlength}<0 ? 0 : $gr{xlength});
			$gr{height} = $gr{yoffset} + ($gr{ylength}<0 ? 0 : $gr{ylength});
			$max{width} = $max{width}>$gr{width}?$max{width}:$gr{width};
			$max{height} = $max{height}>$gr{height}?$max{height}:$gr{height};
		} elsif ( $gr{cmd} eq "refs" ) {
			%refs = %gr;
		}
	}
}

# collect graphics and put them into an array
$begidx = 1;
foreach my $l ( @lines ) {
	$header = "\n<title>$1 - $form{url}</title>\n" if ( $l =~ m|<!--title "(.*?)"-->| );
	if ( $l =~ m"<!--content-->" ) { last; }

	$l =~ s:<!--(\S+)\s+\"([^\"]+)\"-->\n:push @graphics, "cmd=$1&$2"; "":ge;
	$begidx++;
}

# generate the line anchor if required
if ( $form{line} ) {
	splice @lines,$begidx+$form{line}-1,0,"<a name=\"line$form{line}\"/>";
}

&constructGraphics();

$z = 1;

my $images = "";
if ( ! $form{hideimages} ) {
	$images = &insertImages();
} else {
	$au{hideimages} = $form{hideimages};
}

$Lines = join "", @lines;

# construct the $addtourl string
my $addtourl;
map { if ( $au{$_} ne "" ) { $addtourl .= "&amp;$_=$au{$_}"; } } sort keys %au;

$header .= "<style type=\"text/css\">\n";
$header .= ".body { margin-top:0px; margin-left:2ex; }\n";
$header .= " .menuDiv { width:78ex; }\n";
$header .= "  .search { position:relative; top:-6px; }\n";
$header .= " .outerDiv { position:relative; top:-1ex; height:100%; width:100% }\n";
$header .= "  .svgDiv   { position:absolute; top:".$svgpos."px; left:-1ex; z-index:998; }\n";
$header .= "  .imgDiv   { position:absolute; top:0em; }\n";
$header .= "  .imgCenter{ position:absolute; top:0em; text-align:center; margin:0 auto; width:78ex; }\n";
$header .= "  .content  { position:absolute; top:0em; left:0ex; z-index:999; }\n";
$header .= "</style>\n";

if ( $refs{idx} ) {
	if ( ! $form{hidemenu} ) {
		$menudiv =  "<div class=\"menuDiv\"><form action=\"$this\" method=\"GET\"><a href=\"javascript: history.go(-1)\"><img src=\"$config{href_image}/iback.png\" border=\"0\"/></a>";
		$menudiv .= "\n<a href=\"$this\?url=$form{url}$addtourl&amp;index=$refs{prev}\" accesskey=\"p\" rel=\"prev\"><img src=\"$config{href_image}/iprev.png\" border=\"0\"/></a>" if ( $refs{prev} != -1 );
		$menudiv .= "\n<a href=\"$this\?url=$form{url}$addtourl&amp;index=$refs{toc}\" accesskey=\"t\" rel=\"contents\"><img src=\"$config{href_image}/itoc.png\" border=\"0\"/></a>" if ( $refs{toc} != -1 );
		$menudiv .= "\n<a href=\"$this\?url=$form{url}$addtourl&amp;index=$refs{next}\" accesskey=\"n\" rel=\"next\"><img src=\"$config{href_image}/inext.png\" border=\"0\"/></a>" if ( $refs{next} != -1 );
		$menudiv .= "\n<a href=\"$this\?url=$form{url}$addtourl&amp;index=$refs{idx}\" accesskey=\"z\" rel=\"index\"><img src=\"$config{href_image}/iindex.png\" border=\"0\"/></a>" if ( $refs{idx} != -1 );
		$menudiv .= "\n&nbsp;&nbsp;&nbsp;\n<a href=\"../libhyp\" accesskey=\"o\"><img src=\"$config{href_image}/iload.png\" border=\"0\"/></a>";

		$menudiv .= "\n&nbsp;&nbsp;&nbsp;\n";
		$menudiv .= "<input type=\"hidden\" name=\"hideimages\" value=\"$form{hideimages}\"/>\n" if ( $form{hideimages} );
		$menudiv .= "<input type=\"hidden\" name=\"svg\" value=\"$form{svg}\"/>\n" if ( $form{svg} ne "" );
		$menudiv .= "<input type=\"hidden\" name=\"url\" value=\"$form{durl}\"/>\n";
		$menudiv .= "<input class=\"search\" accesskey=\"s\" type=\"text\" name=\"q\" width=\"10\" value=\"$form{q}\"/></form>\n";
		$menudiv .= "</div>\n";
	} else {
		$menudiv = "";
		$addtourl .= "&amp;hidemenu=$form{hidemenu}";
	}

	$header .= "<link href=\"$this\?url=$form{url}$addtourl&amp;index=$refs{prev}\" rel=\"prev\"/>\n" if ( $refs{prev} != -1 );
	$header .= "<link href=\"$this\?url=$form{url}$addtourl&amp;index=$refs{toc}\" rel=\"contents\"/>\n" if ( $refs{toc} != -1 );
	$header .= "<link href=\"$this\?url=$form{url}$addtourl&amp;index=$refs{next}\" rel=\"next\"/>\n" if ( $refs{next} != -1 );
	$header .= "<link href=\"$this\?url=$form{url}$addtourl&amp;index=$refs{idx}\" rel=\"index\"/>\n" if ( $refs{idx} != -1 );
}

# effects
$Lines =~ s"<!--ef 0x([0-9a-fA-F][0-9a-fA-F])-->"&effects(hex($1))"gem;

$Lines =~ s|<!--content-->\n|$images<div class="content"><pre>|m;
$Lines =~ s'<!--/content-->'&effects(0)."</pre></div>"'me;  # close all effect tags

# make <a> links valid to our location
sub emitLink {
	my ($href,$text) = @_;

	my $addua = $addtourl;

	# get the line number and remove from the link
	$href =~ s|\&line=([-\d]\d*)||g;
	my ($line) = $1;
	my ($url) = $form{url};

	# extern links
	if ( $href =~ s|extern=([^\\/]+)[\\/]?(.*)||g ) {
		my ($hyp, $node ) = ($1, $2);
		$au{node} = uri_escape($node);

		# in case we are in an archive then set mask rather then URL
		if ( $au{mask} ) {
			$au{mask} = $hyp;
		} else {
			$url =~ s![^/]+$!$hyp!;
		}

		$addua = "";
		map { if ( $au{$_} ne "" ) { $addua .= "&amp;$_=$au{$_}"; } } sort keys %au;
	}

	$href =~ s|\&|\&amp;|gm; # xml & -> &amp;
	$href .= "&amp;line=$line#line$line" if ( $line > 1 );
	$href = "&amp;$href" if ( $href ne "" );

	"<a href=\"$this\?url=${url}${addua}${href}\">$text</a>"
}

$Lines =~ s|<!--a href=\"(.*?)\"-->(.*?)<!--/a-->|emitLink($1,$2);|gem;

# HTML links (it is worth it in HTML browser ;)
$Lines =~ s|(\s)((https?\|ftp):/[^;:,\)\]\}\"\'<>\n]+)(\.\s)*|$1<a href="$2">$2</a>|gs;
$Lines =~ s|(\s)([a-z]+[a-z0-9.\-_]+\@[a-z0-9.\-_]*[a-z])([;:,\.\]\)\}\"\'<>]*\s)|$1<a href="mailto:$2">$2</a>$3|gim;

# strip the remaining unhandled tags
$Lines =~ s|<!--.*?-->||gm;

# xBF -> &trade;
$Lines =~ s|[\xbf]|\&trade;|g;

# strip and non XML characters
$Lines =~ s|([\x0-\x9\xb\xc\xd-\x1f])|?|gm;

print '<html';
print ' xmlns="http://www.w3.org/1999/xhtml" xmlns:svg="http://www.w3.org/2000/svg"' if ( ! $form{html} );
print ">\n";

print '<head>'."\n";
print   '<meta http-equiv="Content-Type" content="'.$form{ContentType};
print   '; charset='.$form{dstenc} if ( $form{dstenc} );
print "\"/>$header</head>\n";

print "<body class=\"body\">\n$menudiv<div class=\"outerDiv\">\n";

if ( $dosvg ) {
	if ( $form{html} ) {
		print "\n\n<!-- WARNING!!!\n";
		print "     The browser doesn't support 'application/xhtml\+xml' content type which is needed\n";
		print "     to support embedded SVG graphics. Remove the 'svg=$form{svg}' argument to display content.\n-->\n\n\n";
	} elsif ( $#graphs != -1 ) {
		print '<div class="svgDiv">'."\n";
		print ' <svg:svg version="1.1" baseProfile="tiny"'." width=\"".($max{width}*$form{ex}+1)."ex\""." height=\"".($max{height}*$form{em}+1)."em\">\n";
		print "  <svg:defs>\n";
		print '   <svg:marker id="arrowbeg" viewBox="0 0 10 20" refX="2" refY="10" markerUnits="strokeWidth" markerWidth="15" markerHeight="15" orient="auto">'."\n";
		print '     <svg:path d="M 0 10 L 10 0 M 0 10 L 10 20" fill="black" stroke="black"/>'."\n";
		print '   </svg:marker>'."\n";
		print '   <svg:marker id="arrowend" viewBox="0 0 10 20" refX="8" refY="10" markerUnits="strokeWidth" markerWidth="15" markerHeight="15" orient="auto">'."\n";
		print '     <svg:path d="M 10 10 L 0 0 M 10 10 L 0 20" fill="black" stroke="black"/>'."\n";
		print '   </svg:marker>'."\n";
		print "  </svg:defs>\n";
		print '  <svg:g stroke="black">'."\n";
		print @graphs;
		print '  </svg:g>'."\n";
		print ' </svg:svg>'."\n";
		print '</div>'."\n";
	}
}

print $Lines;
# map { print "$_ -> $ENV{$_}\n"; } keys %ENV;
# map { print "$_\n"; } ( $0, $1, $2);
print "</div></body></html>";

