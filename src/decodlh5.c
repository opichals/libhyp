/*
 * libhyp: ST-Guide HYPertext file handling library
 * Copyright (c) 2005-2008 Standa Opichal / JAY Software
 *
 * This file was taken/assembled from the lha100 sources.
 * The aim was to get a single .c file that is able to decompress
 * the lh5 compressed data with no additions. There were no
 * adjustments that would change the (de)compression method in
 * any way.
 *
 * The LHA algorithms and this file contents is licensed
 * according to the original LHA license which does not
 * match other source files of this application.
 *
 * Debian.org LHA package license document:
 * http://packages.debian.org/changelogs/pool/non-free/l/lha/lha_1.14i-10/copyright
 */

/*    Copyright (C) MCMLXXXIX Yooichi.Tagawa                      */
/*    Modified                Nobutaka Watazaki                   */
/*                   Thanks to H.Yoshizaki. (MS-DOS LHarc)        */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define fatal_error(a)


#if defined(__STDC__) || defined(AIX)
/*#include <limits.h>*/
#else
#ifndef CHAR_BIT
#define CHAR_BIT  8
#endif
#ifndef UCHAR_MAX
#define UCHAR_MAX ((1<<(sizeof(unsigned char)*8))-1)
#endif
#ifndef USHRT_MAX
#define USHRT_MAX ((1<<(sizeof(unsigned short)*8))-1)
#endif
#ifndef SHRT_MAX
#define SHRT_MAX ((1<<(sizeof(short)*8-1))-1)
#endif
#ifndef SHRT_MIN
#define SHRT_MIN (SHRT_MAX-USHRT_MAX)
#endif
#endif /* not __STDC__ */


struct decode_option {
	unsigned short (*decode_c)(void);
	unsigned short (*decode_p)(void);
#if defined(__STDC__) || defined(AIX)
	void (*decode_start)(void);
#else
	int (*decode_start)(void);
#endif
};

/* from slide.c */
#define MAX_DICBIT    13 
#define MAX_DICSIZ (1 << MAX_DICBIT)
#define MATCHBIT   8    /* bits for MAXMATCH - THRESHOLD */
#define MAXMATCH 256    /* formerly F (not more than UCHAR_MAX + 1) */
#define THRESHOLD  3    /* choose optimal value */

/* from huf.c */
#define NC (UCHAR_MAX + MAXMATCH + 2 - THRESHOLD)
			/* alphabet = {0, 1, 2, ..., NC - 1} */
#define CBIT 9  /* $\lfloor \log_2 NC \rfloor + 1$ */
#define USHRT_BIT 16	/* (CHAR_BIT * sizeof(ushort)) */


#define NP (MAX_DICBIT + 1)
#define NT (USHRT_BIT + 3)
#define PBIT 4  /* smallest integer such that (1 << PBIT) > NP */
#define TBIT 5  /* smallest integer such that (1 << TBIT) > NT */
/*#if NT > NP
	#define NPT NT
#else
	#define NPT NP
#endif*/
#define NPT 0x80

unsigned short left[2 * NC - 1], right[2 * NC - 1];
unsigned char c_len[NC], pt_len[NPT];
unsigned short c_freq[2 * NC - 1], c_table[4096], c_code[NC],
	   p_freq[2 * NP - 1], pt_table[256], pt_code[NPT],
	   t_freq[2 * NT - 1];
static unsigned short blocksize;



/***** make TABLE */

#define error(a,b)

static short c, n, tblsiz, len, depth, maxdepth, avail;
static unsigned short codeword, bit, *tbl;
static unsigned char *blen;

static short mktbl(void)
{
  short i;

  if (len == depth) {
    while (++c < n)
      if (blen[c] == len) {
	i = codeword;  codeword += bit;
	if (codeword > tblsiz) error(BROKENARC, "Bad table (1)");
	while (i < codeword) tbl[i++] = c;
	return c;
      }
    c = -1;  len++;  bit >>= 1;
  }
  depth++;
  if (depth < maxdepth) {
    (void) mktbl();  (void) mktbl();
  } else if (depth > USHRT_BIT) {
    error(BROKENARC, "Bad table (2)");
  } else {
    if ((i = avail++) >= 2 * n - 1) error(BROKENARC, "Bad table (3)");
    left[i] = mktbl();  right[i] = mktbl();
    if (codeword >= tblsiz) error(BROKENARC, "Bad table (4)");
    if (depth == maxdepth) tbl[codeword++] = i;
  }
  depth--;
  return i;
}

void make_table( short nchar, unsigned char bitlen[], short tablebits, unsigned short table[] )
{
  n = avail = nchar;  blen = bitlen;  tbl = table;
  tblsiz = 1U << tablebits;  bit = tblsiz / 2;
  maxdepth = tablebits + 1;
  depth = len = 1;  c = -1;  codeword = 0;
  (void) mktbl();  /* left subtree */
  (void) mktbl();  /* right subtree */
  if (codeword != tblsiz) error(BROKENARC, "Bad table (5)");
}


/***** end of make table *****/

int unpackable;
unsigned long origsize, compsize;
unsigned short dicbit;
unsigned short maxmatch;
unsigned long count;
unsigned short loc;
unsigned char *text;
int prev_char;


/**** CRCIO *****/

extern int prev_char;
#ifdef EUC
extern int euc_mode;
extern int generic_format;
#endif

long reading_size;

#define CRCPOLY  0xA001  /* CRC-16 */
#define UPDATE_CRC(c) \
	crc = crctable[(crc ^ (c)) & 0xFF] ^ (crc >> CHAR_BIT)

unsigned char *infileptr;
unsigned char *outfileptr;
unsigned short crc, bitbuf;
static unsigned short dicsiz;

static unsigned short crctable[UCHAR_MAX + 1];
static unsigned char  subbitbuf, bitcount;
#ifdef EUC
static int putc_euc_cache;
#endif
static int getc_euc_cache;


void make_crctable(void)
{
	unsigned int i, j, r;

	for (i = 0; i <= UCHAR_MAX; i++) {
		r = i;
		for (j = 0; j < CHAR_BIT; j++)
			if (r & 1) r = (r >> 1) ^ CRCPOLY;
			else       r >>= 1;
		crctable[i] = r;
	}
}


#ifdef NEED_INCREMENTAL_INDICATOR
extern int quiet;
extern int indicator_count;
extern int indicator_threshold;
static void
put_indicator( long int count )
{
	if (!quiet && indicator_threshold)
	{
		while ( count > indicator_count) {
			putchar ('o');
			fflush (stdout);
			indicator_count += indicator_threshold;
		}
	}
}
#endif

unsigned short calccrc( unsigned char *p, unsigned int n )
{
	reading_size += n;
#ifdef NEED_INCREMENTAL_INDICATOR
	put_indicator( reading_size );
#endif
	while (n-- > 0) UPDATE_CRC(*p++);
	return crc;
}

/* Shift bitbuf n bits left, read n bits */
void fillbuf( unsigned char n )
{
  while (n > bitcount) {
    n -= bitcount;
    bitbuf = (bitbuf << bitcount) + (subbitbuf >> (CHAR_BIT - bitcount));
    if (compsize != 0) {
      compsize--;  subbitbuf = *infileptr++;
    } else subbitbuf = 0;
    bitcount = CHAR_BIT;
  }
  bitcount -= n;
  bitbuf = (bitbuf << n) + (subbitbuf >> (CHAR_BIT - n));
  subbitbuf <<= n;
}

unsigned short getbits( unsigned char n )
{
	unsigned short x;

	x = bitbuf >> (2 * CHAR_BIT - n);  fillbuf(n);
	return x;
}


void fwrite_crc( unsigned char *p, int n, unsigned char **fp )
{
  calccrc(p,n);

  if ( fp )
    {
	  memcpy( *fp, p, n );
	  (*fp) += n;

	  /* if (fwrite( p, 1, n, fp) < n)
	    fatal_error("File write error\n");*/
    }
}

void init_code_cache(void)	/* called from copyfile() in util.c */
{
#ifdef EUC
	putc_euc_cache = EOF;
#endif
	getc_euc_cache = EOF;
}

void init_getbits(void)
{
	bitbuf = 0;  subbitbuf = 0;  bitcount = 0;
	fillbuf(2 * CHAR_BIT);
#ifdef EUC
	putc_euc_cache = EOF;
#endif
}

void init_putbits(void)
{
	bitcount = CHAR_BIT;  subbitbuf = 0;
	getc_euc_cache = EOF;
}

#ifdef EUC
void
putc_euc( int c, FILE *fd )
{
  int d;

  if (putc_euc_cache == EOF)
    {
      if (!euc_mode || c < 0x81 || c > 0xFC)
        {
          putc(c, fd);
          return;
        }
      if (c >= 0xA0 && c < 0xE0)
        {
          putc(0x8E, fd);      /* single shift */
          putc(c, fd);
          return;
        }
      putc_euc_cache = c;      /* save first byte */
      return;
    }
  d = putc_euc_cache;
  putc_euc_cache = EOF;
  if (d >= 0xA0)
    d -= 0xE0 - 0xA0;
  if (c > 0x9E)
    {
      c = c - 0x9F + 0x21;
      d = (d - 0x81) * 2 + 0x22;
    }
  else
    {
      if (c > 0x7E)
        c --;
      c -= 0x1F;
      d = (d - 0x81) * 2 + 0x21;
    }
  putc(0x80 | d, fd);
  putc(0x80 | c, fd);
}
#endif


/***** decoding *****/

static void read_pt_len( short nn, short nbit, short i_special )
{
	short i, c, n;

	n = getbits(nbit);
	if (n == 0) {
		c = getbits(nbit);
		for (i = 0; i < nn; i++) pt_len[i] = 0;
		for (i = 0; i < 256; i++) pt_table[i] = c;
	} else {
		i = 0;
		while (i < n) {
			c = bitbuf >> (16 - 3);
			if (c == 7) {
				unsigned short mask = 1 << (16 - 4);
				while (mask & bitbuf) {  mask >>= 1;  c++;  }
			}
			fillbuf((c < 7) ? 3 : c - 3);
			pt_len[i++] = c;
			if (i == i_special) {
				c = getbits(2);
				while (--c >= 0) pt_len[i++] = 0;
			}
		}
		while (i < nn) pt_len[i++] = 0;
		make_table(nn, pt_len, 8, pt_table);
	}
}

static void read_c_len(void)
{
	short i, c, n;

	n = getbits(CBIT);
	if (n == 0) {
		c = getbits(CBIT);
		for (i = 0; i < NC; i++) c_len[i] = 0;
		for (i = 0; i < 4096; i++) c_table[i] = c;
	} else {
		i = 0;
		while (i < n) {
			c = pt_table[bitbuf >> (16 - 8)];
			if (c >= NT) {
				unsigned short mask = 1 << (16 - 9);
				do {
					if (bitbuf & mask) c = right[c];
					else               c = left [c];
					mask >>= 1;
				} while (c >= NT);
			}
			fillbuf(pt_len[c]);
			if (c <= 2) {
				if      (c == 0) c = 1;
				else if (c == 1) c = getbits(4) + 3;
				else             c = getbits(CBIT) + 20;
				while (--c >= 0) c_len[i++] = 0;
			} else c_len[i++] = c - 2;
		}
		while (i < NC) c_len[i++] = 0;
		make_table(NC, c_len, 12, c_table);
	}
}

unsigned short decode_c_st1(void)
{
	unsigned short j, mask;

	if (blocksize == 0) {
		blocksize = getbits(16);
		read_pt_len(NT, TBIT, 3);
		read_c_len();
		read_pt_len(NP, PBIT, -1);
	}
	blocksize--;
	j = c_table[bitbuf >> 4];
	if (j < NC) fillbuf(c_len[j]);
	else {
		fillbuf(12);  mask = 1 << (16 - 1);
		do {
			if (bitbuf & mask) j = right[j];
			else               j = left [j];
			mask >>= 1;
		} while (j >= NC);
		fillbuf(c_len[j] - 12);
	}
	return j;
}

unsigned short decode_p_st1(void)
{
	unsigned short j, mask;

	j = pt_table[bitbuf >> (16 - 8)];
	if (j < NP) fillbuf(pt_len[j]);
	else {
		fillbuf(8);  mask = 1 << (16 - 1);
		do {
			if (bitbuf & mask) j = right[j];
			else               j = left [j];
			mask >>= 1;
		} while (j >= NP);
		fillbuf(pt_len[j] - 8);
	}
	if (j != 0) j = (1 << (j - 1)) + getbits(j - 1);
	return j;
}

void decode_start_st1(void)
{
	init_getbits();
	blocksize = 0;
}


int decode_lh5( char *infp, char *outfp, long original_size, long packed_size )
{
	int i, j, k, c, dicsiz1, offset;

	infileptr = (unsigned char *)infp;
	outfileptr = (unsigned char *)outfp;
	dicbit = 13;
	origsize = original_size;
	compsize = packed_size;

	crc = 0;
	prev_char = -1;
	dicsiz = 1 << dicbit;
	text = (unsigned char *)malloc(dicsiz);
	if (text == NULL)
		return 0;
	memset(text, ' ', dicsiz);
	decode_start_st1();
	dicsiz1 = dicsiz - 1;
	offset = 0x100 - 3;
	count = 0;  loc = 0;
	while (count < origsize) {
		c = decode_c_st1();
		if (c <= UCHAR_MAX) {
			text[loc++] = c;
			if (loc == dicsiz) {
				fwrite_crc(text, dicsiz, &outfileptr);
				loc = 0;
			}
			count++;
		} else {
			j = c - offset;
			i = (loc - decode_p_st1() - 1) & dicsiz1;
			count += j;
			for (k = 0; k < j; k++) {
				c = text[(i + k) & dicsiz1];
				text[loc++] = c;
				if (loc == dicsiz) {
					fwrite_crc(text, dicsiz, &outfileptr);
					loc = 0;
				}
			}
		}
	}
	if (loc != 0) {
		fwrite_crc(text, loc, &outfileptr);
	}
	free(text);

	return crc;
}

