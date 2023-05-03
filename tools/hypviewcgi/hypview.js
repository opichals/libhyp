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


const MAX_WIDTH = 74;
var hypfile;
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
			$images += `<div class="img" style="left:${Math.round((hyp_width/MAX_WIDTH)*(args.xoffset-1))}; z-index:${z};"><div><pre>`;
		}

		// yoffset number of newlines
		let $count = args.yoffset;
		while ( $count-- > 0 ) { $images += "<br/>"; }

		// @limage additional newlines
		if ( args.type === "limage" ) {
			let $count = Math.round((args.height+15)/16);
			let $limgnl = "";
			while ( $count-- > 0 ) { $limgnl += "\n"; }
			lines.splice($begidx+args.ytextoffset,0,$limgnl); $begidx += 1;
		}

		$images += `<img src="${dehyp(hypfile, args.index, true)}"/></pre></div></div>`;

		z++;
	}

	return $images;
}

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
				$offset += Math.round((gr.height+15)/16);
			}

			limg_args.push(gr);
		} else if ( gr.cmd === "box" ) {
            gr.pattern = parseInt(gr.pattern, 10);

			g  = `  <rect x="${gr.xoffset}" y="${gr.yoffset}"`;
			g += ` width="${gr.width}" height="${gr.height}" style="`;
			if ( gr.pattern == 8 ) {
				g += "fill:rgb(0,0,0);\"";
			} else if ( gr.pattern != 0 ) {
				let grey = (8-gr.pattern)*32;
				g += `fill:rgb(${grey},${grey},${grey});"`;
			} else {
				g += "fill:none;\"";
			}
			if ( gr.rbox != 0 ) {
				g += ` rx="${Math.min(gr.width, gr.height)/4}" ry="${Math.min(gr.width, gr.height)/8}"`;
			}
			gr.svg = `${g}/>`;
			graphs.push(gr.svg);

			gr.width = gr.xoffset+gr.width;
			gr.height = gr.yoffset+gr.height;
			$max.width = $max.width>gr.width?$max.width:gr.width;
			$max.height = $max.height>gr.height?$max.height:gr.height;
		} else if ( gr.cmd === "line" ) {
            gr.xlength = parseInt(gr.xlength, 10);
            gr.ylength = parseInt(gr.ylength, 10);
            gr.style = parseInt(gr.style, 10);
            gr.attribs = parseInt(gr.attribs, 10);

			g  = `  <line x1="${gr.xoffset}" y1="${gr.yoffset}"`;
			g += ` x2="${(gr.xoffset+gr.xlength)}" y2="${(gr.yoffset+gr.ylength)}"`;
			if ( gr.style == 2 ) {
				g += " stroke-dasharray=\"0.8,0.2\"";
			} else if ( gr.style == 3 ) {
				g += " stroke-dasharray=\"0.2,0.5\"";
			} else if ( gr.style == 4 ) {
				g += " stroke-dasharray=\"0.6,0.1,0.1,0.1\"";
			} else if ( gr.style == 5 ) {
				g += " stroke-dasharray=\"0.6,0.6\"";
			} else if ( gr.style == 6 ) {
				g += " stroke-dasharray=\"0.3,0.1,0.2,0.1,0.2,0.1,0.3,0.1\"";
			} else if ( gr.style == 7 ) {
				g += " stroke-dasharray=\"0.1,0.1\"";
			}
			if ( gr.attribs & 1 ) {
				g += ' marker-start="url(#arrowbeg)"';
			}
			if ( gr.attribs & 2 ) {
				g += ' marker-end="url(#arrowend)"';
			}
			gr.svg = `${g}/>`;
			graphs.push(gr.svg);

			gr.width = gr.xoffset + (gr.xlength<0 ? 0 : gr.xlength);
			gr.height = gr.yoffset + (gr.ylength<0 ? 0 : gr.ylength);
			$max.width = $max.width>gr.width?$max.width:gr.width;
			$max.height = $max.height>gr.height?$max.height:gr.height;
		}
	}
}

// make <a> links valid to our location
function emitLink(file, href, text) {
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

	return `<a href="${file}#${href}">${text}</a>`;
}

function htmllify(file, lines, lineNumber) {

$begidx = 1;

// collect graphics and put them into an array
let graphics = [];
for(let i=0; i<lines.length; i++) {
    let $l = lines[i];

    let title = $l.match(/<!--title "(.*?)"-->/);
    if (title) {
        const $header = `${title[1]} - hypview: ${hypfile}`;
        document.title = $header.replace(/[\x80-\xff]/g, (m) => hypenc[m[0].charCodeAt(0)-128]); // TODO: cleanup
    }
	if ( $l.match(/<!--content-->/) ) { break; }

	lines[i] = $l.replace(/<!--(\S+)\s+\"([^\"]+)\"-->\n/g, (m, cmd, args) => (graphics.push(`cmd=${cmd}&${args}`), ''));
	$begidx++;
}

// generate the line anchor if required
if ( lineNumber ) {
    lineNumber = parseInt(lineNumber, 10);
    lines.splice($begidx+lineNumber-1,0,`<a name="line${lineNumber}" class="anchor"/>`);
}

len = lines.length;
$max = {};
graphs = [];
limg_args = [];
constructGraphics(graphics);

const $images = insertImages(lines);

$Lines = lines.join('');

$Lines = $Lines.replace(/<!--a href="(.*?)"-->(.*?)<!--\/a-->/gm, (m, href, text) => emitLink(file,href,text));

// effects
$Lines = $Lines.replace(/<!--ef (0x[0-9a-fA-F][0-9a-fA-F])-->/gm, (m, eff) => effects(parseInt(eff, 16)));

$Lines = $Lines.replace(/<!--content-->\n/m, $images);
$Lines = $Lines.replace(/<!--\/content-->\n/m, () => effects(0)); // close all effect tags

// HTML links (it is worth it in HTML browser ;)
$Lines = $Lines.replace(/(\s)((https?|ftp):\/[^;:,\)\]\}\"\'<>\n]+)[\.\s]?/g, '$1<a href="$2">$2</a>');
$Lines = $Lines.replace(/(\s)([a-z]+[a-z0-9.\-_]+\@[a-z0-9.\-_]*[a-z])([;:,\.\]\)\}\"\'<>]*\s)/gim, '$1<a href="mailto:$2">$2</a>$3');

// strip the remaining unhandled tags
$Lines = $Lines.replace(/<!--.*?-->/gm, '');

// xBF -> &trade;
$Lines = $Lines.replace(/\xbf/g, '&trade;');

// apply character encoding
$Lines = $Lines.replace(/[\x80-\xff]/g, (m) => hypenc[m[0].charCodeAt(0)-128]);

// strip and non XML characters
// $Lines =~ s|([\x0-\x9\xb\xc\xd-\x1f])|?|gm;

document.getElementById('graphics').style.visibility = "hidden";
document.getElementById('graphics').innerHTML = graphs.join('');

return $Lines;
}

