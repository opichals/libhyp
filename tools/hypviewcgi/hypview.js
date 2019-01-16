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
	
	return $eff;
}


let $max = {};
let limg_args;
let $begidx;

function insertImages(lines) {
    let z = 1;
    let $images = '';

	// fixup the @(l)image ypositions positions
	for (let args of limg_args) {
		if ( args.xoffset == 0 ) {
			// centered image
			$images += `<div class="imgCenter" style="z-index:${z};"><div><pre>`;
		} else {
			// xoffset positioned image
			$images += `<div class="imgDiv" style="left:${Math.round(args.xoffset*1.35)}ex; z-index:${z};"><div><pre>`;
		}

		// yoffset number of newlines
		let $count = args.yoffset;
		while ( $count-- > 0 ) { $images += "<br/>"; }

		// @limage additional newlines
		if ( args.type === "limage" ) {
			let $count = ((args.height+15)/16) - 1;
			let $limgnl = "";
			while ( $count-- >= -1 ) { $limgnl += "\n"; }
			lines.splice($begidx+args.ytextoffset,0,$limgnl); $begidx += 1;
		}

		$images += `<img src="${dehyp(hypfile, args.index, true)}"/></pre></div></div>`;

		z++;
	}

	return $images;
}

// the following needs to be somehow calibrated (the values are
// just empirical from OS X browser versions (default font settings)
let ex = 0; // $form.ex;
if ( ex == 0 ) {
	ex = 1; //# Safari
	ex = 1.077; //# Firefox
	ex = 1.085; //# Chrome OSX
}
let em = 0; // $form.em;
if ( em == 0 ) {
	em = 0.9376; $svgpos = -2;  //# Safari
	em = 0.8124; $svgpos = -4; //# Opera
	em = 0.875; $svgpos = -4; //# Firefox 3.x
	em = 0.935; $svgpos = -4; //# Chrome OSX
}
// $form{ex} = 1; $form{em} = 1; $svgpos = 0;


let graphs;
function constructGraphics(graphics) {
    let $offset = 0;

	// fixup the @(l)image ypositions positions
	for(let $gr of graphics) {
		let g;
        let gr = $gr.split('&').reduce((gr, arg) => { let kv = arg.split('='); return {...gr, [kv[0]]: kv[1]}; }, {});
		// adjust according to the @limage offset
        gr.xoffset = parseInt(gr.xoffset, 10);
        gr.yoffset = parseInt(gr.yoffset, 10);
        gr.width = parseInt(gr.width, 10);
        gr.height = parseInt(gr.height, 10);

		gr.yoffset += $offset;

		if ( gr.cmd === "img" ) {
			// compute the graphics offset caused by @limage lines
			if ( gr.type === "limage" ) {
				gr.ytextoffset += gr.yoffset - $offset;

                gr.height = parseInt(gr.height, 10);
				let $count = (gr.height+15)/16;
				$offset += $count + 1;
			}

			limg_args.push(gr);
		} else if ( gr.cmd === "box" ) {
            gr.pattern = parseInt(gr.pattern, 10);

			g  = `  <rect x="${gr.xoffset*ex}ex" y="${gr.yoffset*em+1}em"`;
			g += ` width="${gr.width*ex}ex" height="${gr.height*em}em" style="`;
			if ( gr.pattern == 8 ) {
				g += "fill:rgb(0,0,0);\"";
			} else if ( gr.pattern != 0 ) {
				let grey = (8-gr.pattern)*32;
				g += `fill:rgb(${grey},${grey},${grey});"`;
			} else {
				g += "fill:none;\"";
			}
			if ( gr.rbox != 0 ) {
				g += ` rx="${(gr.width+gr.height)/2}"`;
			}
			graphs.push(`${g}></rect>\n`);

			gr.width = gr.xoffset+gr.width;
			gr.height = gr.yoffset+gr.height;
			$max.width = $max.width>gr.width?$max.width:gr.width;
			$max.height = $max.height>gr.height?$max.height:gr.height;
		} else if ( gr.cmd === "line" ) {
            gr.xlength = parseInt(gr.xlength, 10);
            gr.ylength = parseInt(gr.ylength, 10);
            gr.style = parseInt(gr.style, 10);
            gr.attribs = parseInt(gr.attribs, 10);

			g  = `  <line x1="${gr.xoffset*ex}ex" y1="${gr.yoffset*em+1}em"`;
			g += ` x2="${gr.xoffset+gr.xlength*ex}ex" y2="${(gr.yoffset+gr.ylength)*em+1}em"`;
			if ( gr.style == 2 ) {
				g += " stroke-dasharray=\"8,2\"";
			} else if ( gr.style == 3 ) {
				g += " stroke-dasharray=\"2,5\"";
			} else if ( gr.style == 4 ) {
				g += " stroke-dasharray=\"6,1,1,1\"";
			} else if ( gr.style == 5 ) {
				g += " stroke-dasharray=\"6,6\"";
			} else if ( gr.style == 6 ) {
				g += " stroke-dasharray=\"3,1,2,1,2,1,3,1\"";
			} else if ( gr.style == 7 ) {
				g += " stroke-dasharray=\"1,1\"";
			}
			if ( gr.attribs & 1 ) {
				g += ' marker-start="url(#arrowbeg)"';
			}
			if ( gr.attribs & 2 ) {
				g += ' marker-end="url(#arrowend)"';
			}
			graphs.push(`${g}></line>\n`);

			gr.width = gr.xoffset + (gr.xlength<0 ? 0 : gr.xlength);
			gr.height = gr.yoffset + (gr.ylength<0 ? 0 : gr.ylength);
			$max.width = $max.width>gr.width?$max.width:gr.width;
			$max.height = $max.height>gr.height?$max.height:gr.height;
		}
	}
}

// make <a> links valid to our location
function emitLink(href, text) {
	// get the line number and remove from the link
    let line = 1;
	href = href.replace(/\&line=([-\d]\d*)/g, (m, l) => (line = parseInt(l, 10), ''));

	// // extern links
	// if ( $href =~ s|extern=([^\\/]+)[\\/]?(.*)||g ) {
	// 	my ($hyp, $node ) = ($1, $2);
	// 	$au{node} = uri_escape($node);

	// 	# in case we are in an archive then set mask rather then URL
	// 	if ( $au{mask} ) {
	// 		$au{mask} = $hyp;
	// 	} else {
	// 		$url =~ s![^/]+$!$hyp!;
	// 	}
	// }

    if ( line > 1 ) href += `&line=${line}#line${line}`;
    if ( href ) href = `&${href}`;

	return `<a href="#${href}">${text}</a>`;
}

function htmllify(lines) {

$begidx = 1;

// collect graphics and put them into an array
let graphics = [];
for(let i=0; i<lines.length; i++) {
    let $l = lines[i];
    if ( $l.match(/<!--title "(.*?)"-->/) ) {
        // FIXME $header = "\n<title>$1 - $form{url}</title>\n";
    }
	if ( $l.match(/<!--content-->/) ) { break; }

	lines[i] = $l.replace(/<!--(\S+)\s+\"([^\"]+)\"-->\n/g, (m, cmd, args) => (graphics.push(`cmd=${cmd}&${args}`), ''));
	$begidx++;
}

// # generate the line anchor if required
// if ( $form{line} ) {
// 	splice lines,$begidx+$form{line}-1,0,"<a name=\"line$form{line}\"/>";
// }

$max = {};
graphs = [];
limg_args = [];
constructGraphics(graphics);

const $images = insertImages(lines);

$Lines = lines.join('');

$Lines = $Lines.replace(/<!--a href="(.*?)"-->(.*?)<!--\/a-->/gm, (m, href, text) => emitLink(href,text));

// effects
$Lines = $Lines.replace(/<!--ef (0x[0-9a-fA-F][0-9a-fA-F])-->/gm, (m, eff) => effects(parseInt(eff, 16)));

$Lines = $Lines.replace(/<!--content-->\n/m, $images);
$Lines = $Lines.replace(/<!--\/content-->\n/m, () => effects(0)); // close all effect tags

console.log($Lines);
// HTML links (it is worth it in HTML browser ;)
$Lines = $Lines.replace(/(\s)((https?\|ftp):\/[^;:,\)\]\}\"\'<>\n]+)[\.\s]?/g, '$1<a href="$2">$2</a>');
$Lines = $Lines.replace(/(\s)([a-z]+[a-z0-9.\-_]+\@[a-z0-9.\-_]*[a-z])([;:,\.\]\)\}\"\'<>]*\s)/gim, '$1<a href="mailto:$2">$2</a>$3');

// strip the remaining unhandled tags
$Lines = $Lines.replace(/<!--.*?-->/gm, '');

// xBF -> &trade;
$Lines = $Lines.replace(/\xbf/g, '&trade;');

// strip and non XML characters
// $Lines =~ s|([\x0-\x9\xb\xc\xd-\x1f])|?|gm;

document.getElementById('svg').setAttribute('width', ($max.width*ex+1)+'ex');
document.getElementById('svg').setAttribute('height', ($max.height*em+1)+'em');

document.getElementById('graphics').innerHTML = graphs;

return $Lines;
}



