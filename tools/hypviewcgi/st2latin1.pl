#!/usr/bin/perl -np

my ($to_latin1) = "";
$to_latin1 .= "\xe7\xfc\xe9\xe2\xe4\xe0\xe5\xc7\xea\xeb\xea\xef\xee\xec\xc4\xc5";
$to_latin1 .= "\xc9\xe6\xc6\xf4\xf6\xf2\xfb\xf9\xff\xf6\xdc\xa2\xa3\xa5\xdf\x9f";
$to_latin1 .= "\xe1\xed\xf3\xfa\xf1\xd1\xaa\xba\xbf\xa9\xac\xbd\xbc\xa1\xab\xbd";
$to_latin1 .= "\xe3\xf5\xd8\xf8\xb4\xb5\xc0\xc3\xd5\xa8\xb4\xbb\xbb\xa9\xae\xbf";
$to_latin1 .= "\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf";
$to_latin1 .= "\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf";
$to_latin1 .= "\xe0\xdf\xe2\xe3\xe4\xe5\xb5\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef";
$to_latin1 .= "\xf0\xb1\xf2\xf3\xf4\xf5\xf7\xf7\xb0\xb7\xfa\xfb\xfc\xb2\xb3\xaf";

eval qq{ \$_ =~ tr/\x80-\xff/$to_latin1/ };
