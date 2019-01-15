let $oldeff = 0;
function effects($e) {
	let $ch = $e ^ $oldeff;
	let $ne = ~$e;
	let $eff = "";
	
	// {i}ggg{u}xxx{b}yyy{U}zx{I}zz{B}
	// no optimizations for now here simple close all $oldeff and open the $e
	$ne = $oldeff;
	$ch = $oldeff;
	$ekeep = $e;

	if ($ch & 0x8 && $ne & 0x8 ) {
		$eff += "</span>";
	}
	if ($ch & 0x4 && $ne & 0x4 ) {
		$eff += "</em>";
	}
	if ($ch & 0x2 && $ne & 0x2 ) {
		$eff += "</em>";
	}
	if ($ch & 0x1 && $ne & 0x1 ) {
		$eff += "</b>";
	}

	$ch |= $ekeep;
	if ($ch & 0x1 && $e & 0x1 ) {
		$eff += "<b>";
	}
	if ($ch & 0x2 && $e & 0x2 ) {
		$eff += "<em>";
	}
	if ($ch & 0x4 && $e & 0x4 ) {
		$eff += "<em>";
	}
	if ($ch & 0x8 && $e & 0x8 ) {
		$eff += "<span style=\"text-decoration: underline;\">";
	}
	$oldeff = $e;
	
	return $eff+"<!--ef "+$e+"-->";
}

const $images = '';

function htmllify($Lines) {
// effects
$Lines = $Lines.replace(/<!--ef (0x[0-9a-fA-F][0-9a-fA-F])-->/gm, (m, eff) => effects(parseInt(eff, 16)));

$Lines = $Lines.replace(/<!--content-->\n/m, $images);
$Lines = $Lines.replace(/<!--\/content-->/m, () => effects(0)); // close all effect tags

// HTML links (it is worth it in HTML browser ;)
$Lines = $Lines.replace(/(\s)((https?\|ftp):\/[^;:,\)\]\}\"\'<>\n]+)[\.\s]?/g, '$1<a href="$2">$2</a>');
$Lines = $Lines.replace(/(\s)([a-z]+[a-z0-9.\-_]+\@[a-z0-9.\-_]*[a-z])([;:,\.\]\)\}\"\'<>]*\s)/gim, '$1<a href="mailto:$2">$2</a>$3');

// strip the remaining unhandled tags
$Lines = $Lines.replace(/<!--.*?-->/gm, '');

// xBF -> &trade;
$Lines = $Lines.replace(/\xbf/g, '&trade;');

// strip and non XML characters
// $Lines =~ s|([\x0-\x9\xb\xc\xd-\x1f])|?|gm;

return $Lines;
}



