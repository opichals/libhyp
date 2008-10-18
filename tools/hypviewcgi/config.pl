#
# for http://libhyp.atariforge.org/cgi-bin/hypview.cgi location
#
%config = (
	cache => "cache",
	tmp => "cache/extract",
	href_image => "/image",
	href_openhyp => "/",
	fetch => 'curl -qfL -o $FILE --url $url 2>/dev/null'
#	fetch => 'wget -q -O $FILE -c $url 2>/dev/null'
#	enca => "./enca/bin/enca -Lcs -x"
);

1
