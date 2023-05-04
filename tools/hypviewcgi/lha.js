// from https://github.com/kyz/lha.js
// LICENSE: GPLv3

LHA = {}

/**
 * Reads an LhA archive file from a URL
 *
 * @param {String} url URL to LhA file
 * @param {Function} callback function to call with results of reading archive
 * @see LHA.read()
 */
LHA.readFromURL = function (url, callback) {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", url);
    xhr.responseType = "arraybuffer";
    xhr.onloadend = function () {
        callback(LHA.read(new Uint8Array(xhr.response || 0)));
    }
    xhr.send();
}

/**
 * Reads an LhA archive file
 *
 * @param {Uint8Array} data byte array of data to read as an LhA file
 * @returns {Array} array of entries for each  file in the archive
 */
LHA.read = function (data) {
    function chk(i,len) {if (i + len > data.length) throw "read out of bounds";}
    function u8(i)  { chk(i, 1); return data[i];}
    function u16(i) { chk(i, 2); return data[i] | (data[i+1] << 8);}
    function u32(i) { chk(i, 4); return data[i] | (data[i+1] << 8) | (data[i+2] << 16) | (data[i+3] << 24);}
    function str(i, len) { chk(i, len); return String.fromCharCode.apply(undefined, data.subarray(i, i+len));}

    // file format info: http://fileformats.archiveteam.org/wiki/LHA
    var offset = 0, out = [];
    while ((data.length - offset) >= 24) {
        // check header level is 0-3
        var level = u8(offset + 20);
        if (level > 3) {
            throw "unknown header level " + level;
        }
        if (level === 3 && u16(offset) != 4) {
            throw "header level 3 with unknown word size";
        }

        var time = u32(offset + 15),
            nameLength = u8(offset + 21);
        var entry = {
            packMethod:   str(offset + 2, 5),
            packedLength: u32(offset + 7),
            length:       u32(offset + 11),
            lastModified: (level < 2)
                ? new Date((time >> 25) + 1980, ((time >> 21) & 15) - 1, (time >> 16) & 31,
                           (time >> 11) & 31, (time >> 5) & 63, time & 63) // MS-DOS style
                : new Date(time * 1000), // UNIX style
            name: ""
        };

        // read level 0,1 filename
        if (level < 2 && nameLength > 0) {
            // Amiga LhA adds a file comment to the filename after a null byte
            var name = str(offset + 22, nameLength),
                parts = name.split("\0");
            entry.name = parts[0];
            if (parts.length > 1) {
                entry.comment = parts[1];
            }
        }

        // read the extended headers
        // level 1, 2: [2 bytes header length] [1 byte type] [n bytes data]
        // level 3:    [4 bytes header length] [1 byte type] [n bytes data]
        if (level > 0) {
            var headerOffset = (level === 3) ? 28 : // level 3
                               (level === 2) ? 24 : // level 2
                               u8(offset),          // level 1
                headerSize   = (level === 3) ? 4 : 2,
                readLength   = (level === 3) ? u32 : u16,
                headerLength, directory;
            while ((headerLength = readLength(offset + headerOffset)) > 0) {
                var dataOffset = offset + headerOffset + headerSize + 1,
                    dataLength = headerLength - headerSize - 1;
                switch (u8(dataOffset - 1)) { // extended header type
                case 1:  entry.name    = str(dataOffset, dataLength); break;
                case 2:  directory     = str(dataOffset, dataLength); break;
                case 63: entry.comment = str(dataOffset, dataLength); break;
                }
                headerOffset += headerLength;
            }
            headerOffset += headerSize; // include 0-length terminating header
            if (directory) {
                entry.name = directory.replace(/\xFF/g, "\\") + entry.name;
            }
        }

        // total length of headers
        var headersLength =
            (level < 2)   ? u8(offset) + 2 :  // level 0,1
            (level === 2) ? u16(offset) :     // level 2
                            u32(offset + 24); // level 3

        // fix level 1 packedLength (it adds extended headers length:
        // move that excess from packedLength into headersLength)
        if (level === 1) {
            entry.packedLength -= (headerOffset - headersLength);
            headersLength = headerOffset;
        }

        // create subarray of the compressed data
        entry.data = data.subarray(offset + headersLength,
                                   offset + headersLength + entry.packedLength);
        out.push(entry);

        // advance to next entry
        offset += headersLength + entry.packedLength;
    }
    return out;
}

/**
 * Unpacks a file entry returned from LHA.read()
 *
 * @param {object} entry entry returned from LHA.read()
 * @returns {Uint8Array} decompressed file
 */
LHA.unpack = function (entry) {
    "use strict";
    switch (entry.packMethod) {
    case "-lh0-": // no compression
    case "-lz4-": // no compression
    case "-pm0-": // no compression
    case "-lhd-": // this is a directory, not a file
        return entry.data;

    case "-lh4-":
        return LHA._unpack_lha2(13, entry.data, entry.length);
    case "-lh5-":
        return LHA._unpack_lha2(14, entry.data, entry.length);
    case "-lh6-":
        return LHA._unpack_lha2(16, entry.data, entry.length);
    case "-lh7-":
        return LHA._unpack_lha2(17, entry.data, entry.length);

    }
}

// LHA v2 compression format (-lh4-, -lh5-, -lh6-, -lh7-)
LHA._unpack_lha2 = function (window_bits, input, length) {
    var output    = new Uint8Array(length),
        w         = new Uint8Array(1 << window_bits), // history window
        pretree   = LHA._alloc_tree(20, 7),
        main      = LHA._alloc_tree(510, 9),
        distances = LHA._alloc_tree(window_bits, 7),
        ip        = 0, // index into input[]
        op        = 0, // index into output[]
        wp        = 0, // index into w[]
        wmask     = (1 << window_bits) - 1, // keeps wp within w[] bounds
        bb        = 0, // bit-buffer
        bl        = 0; // number of bits left in the bit buffer

    // history window initially full of spaces
    w.fill(32);

    function peek_bits(n) {while (bl < n) bb = (bb << 8) | input[ip++], bl += 8;
                           return (bb >> (bl - n)) & (1 << n) - 1;}
    function drop_bits(n) {bl -= n;}
    function read_bits(n) {var bits = peek_bits(n); drop_bits(n); return bits;}
    function write_byte(byte) {output[op++] = w[wp++] = byte; wp &= wmask;}

    function read_tree(tree, read_length) {
        // clear code lengths
        for (var i = tree.max_codes; i--;) tree.code_lengths[i] = 0;
        // read how many code lengths will follow
        var bits_needed = Math.ceil(Math.log2(tree.max_codes + 1)),
            n = Math.min(read_bits(bits_needed), tree.max_codes);
        if (n === 0) {
            LHA._build_single_code_tree(tree, read_bits(bits_needed));
        }
        else {
            // otherwise, read each code length using per-tree function
            for (var i = 0; i < n; i++) {
                var skip = read_length(tree.code_lengths, i);
                if (skip) i += skip;
            }
            LHA._build_tree(tree);
        }
    }

    function read_code(tree) {
        var bits = tree.lookup_bits,
            code = tree.lookup[peek_bits(bits)];
        while (code >= tree.max_codes) {
            code = tree.lookup[(code << 1) | (peek_bits(++bits) & 1)];
        }
        drop_bits(tree.code_lengths[code]);
        return code;
    }

    while (op < length) {
        var steps = read_bits(16);

        read_tree(pretree, function (code_lengths, i) {
            code_lengths[i] = read_bits(3);
            if (code_lengths[i] === 7) while (read_bits(1)) code_lengths[i]++;
            if (i === 2) return read_bits(2);
        });

        read_tree(main, function (code_lengths, i) {
            var code = read_code(pretree);
            if (code === 1) return read_bits(4) + 2;
            if (code === 2) return read_bits(9) + 19;
            if (code > 2) code_lengths[i] = code - 2;
        });

        read_tree(distances, function (code_lengths, i) {
            code_lengths[i] = read_bits(3);
            if (code_lengths[i] === 7) while (read_bits(1)) code_lengths[i]++;
        });

        while (steps--) {
            var code = read_code(main);
            if (code < 256) {
                write_byte(code);
            }
            else {
                var len = code - 256 + 3,
                    dist_code = read_code(distances),
                    distance = (dist_code === 0) ? 0 :
                               (dist_code === 1) ? 1 :
                    (1 << (dist_code-1)) + read_bits(dist_code-1);
                var cp = wp - distance - 1;
                while (len--) write_byte(w[cp++ & wmask]);
            }
        }
    }
    return output;
}

LHA._alloc_tree = function (max_codes, lookup_bits) {
    return {
        max_codes:    max_codes,
        lookup_bits:  lookup_bits,
        code_lengths: new Uint8Array(max_codes),
        lookup:       new Uint16Array(Math.max(1 << lookup_bits, max_codes << 1) + (max_codes << 1))
    };
}

LHA._build_tree = function (tree) {
    var max_codes = tree.max_codes,
        code_lengths = tree.code_lengths,
        lookup = tree.lookup,
        lookup_bits = tree.lookup_bits,
        lookup_size = 1 << lookup_bits,
        posn = 0;

    // fill entries for codes short enough for a direct mapping
    for (var b = 1, f = lookup_size >> 1; b <= lookup_bits; b++, f >>= 1) {
        for (var code = 0; code < max_codes; code++) {
            if (code_lengths[code] === b) {
                if (posn + f > lookup_size) throw "overrun";
                for (var i = f; i--;) lookup[posn++] = code;
            }
        }
    }

    if (posn === lookup_size) return;

    // mark remaining lookup entries as "unused"
    for (var i = posn; i < lookup_size; i++) lookup[i] = 0xFFFF;

    var alloc_posn = Math.max(lookup_size >> 1, max_codes);

    // allow codes to be up to {lookup_bits}+16 bits long
    posn <<= 16;
    lookup_size <<= 16;

    // find the longest code length
    var max_bits = 0;
    for (var i = 0; i < max_codes; i++) {
        max_bits = Math.max(max_bits, code_lengths[i]);
    }

    // allocate pairs of code entries starting at {max_codes}
    // upwards that either point to other such high codes, or
    // terminate in a code less than {max_codes} - a table-based
    // tree for all codes that are longer than {lookup_bits}
    for (var b = lookup_bits+1, f = 1<<15; b <= max_bits; b++, f >>= 1) {
        for (var code = 0; code < max_codes; code++) {
            if (code_lengths[code] === b) {
                if (posn >= lookup_size) throw "overrun";

                var leaf = posn >> 16;
                for (var fill = 0; fill < (b - lookup_bits); fill++) {
                    // if this path hasn't been taken yet, "allocate" entries
                    if (lookup[leaf] == 0xFFFF) {
                        lookup[(alloc_posn << 1)    ] = 0xFFFF;
                        lookup[(alloc_posn << 1) | 1] = 0xFFFF;
                        lookup[leaf] = alloc_posn++;
                    }
                    // now follow either that left or right branch
                    leaf = (lookup[leaf] << 1) | ((posn >> (15-fill)) & 1);
                }

                lookup[leaf] = code;
                posn += f;
            }
        }
    }

    if (posn !== lookup_size) throw "incomplete table";
}

LHA._build_single_code_tree = function (tree, code) {
    // any bit sequence decodes to this single code
    var lookup_size = 1 << tree.lookup_bits;
    for (var i = 0; i < lookup_size; i++) {
        tree.lookup[i] = code;
    }
    // and it consumes no bits
    for (var i = 0; i < tree.max_codes; i++) {
        tree.code_lengths[i] = 0;
    }
}
