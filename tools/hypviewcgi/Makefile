#!make -f
#
# This software is licensed under the Lesser GNU General Public License.
# Please, see LICENSE.TXT for further information.
#

top_srcdir = ../..
srcdir     = .

include $(top_srcdir)/CONFIGVARS
HYPVIEWDISTDIR = $(DISTDIR)/tools/hypviewcgi

TARGET = dehyp$(TARGET_SUFFIX) grephyp$(TARGET_SUFFIX)

all: $(TARGET)

CFLAGS = $(OPTS) -I$(top_srcdir)/include $(CROSS_CFLAGS)

# engine binary objects
DEHYP_OBJS = dehyp.o emitpng.o grep.o tools.o
GREPHYP_OBJS = grephyp.o grep.o tools.o

dehyp.o: $(top_srcdir)/include/libhyp.h
grephyp.o: $(top_srcdir)/include/libhyp.h

dehyp: $(DEHYP_OBJS) $(libhyp_dir)/libhyp.a
	$(CC) -o $@ $(DEHYP_OBJS) -L$(libhyp_dir) -lhyp -L$(libpng_dir) -lpng $(CROSS_LDFLAGS)

grephyp: $(GREPHYP_OBJS) $(libhyp_dir)/libhyp.a
	$(CC) -o $@ $(GREPHYP_OBJS) -L$(libhyp_dir) -lhyp $(CROSS_LDFLAGS)

# emscripten
dehyp.html: $(DEHYP_OBJS) $(libhyp_dir)/libhyp.a
	$(CC) -o $@ $(DEHYP_OBJS) -L$(libhyp_dir) -lhyp -L$(libpng_dir) -s USE_ZLIB=1 -s USE_LIBPNG=1 $(CROSS_LDFLAGS)

grephyp.html: $(GREPHYP_OBJS) $(libhyp_dir)/libhyp.a
	$(CC) -o $@ $(GREPHYP_OBJS) -L$(libhyp_dir) -lhyp $(CROSS_LDFLAGS)

strip:
	$(STRIP) --strip-all $(TARGET)

dist:
	mkdir -p $(HYPVIEWDISTDIR)
	cp -r $(srcdir)/README $(HYPVIEWDISTDIR)
	cp -r $(srcdir)/Makefile $(HYPVIEWDISTDIR)
	cp -r $(srcdir)/dehyp.c $(HYPVIEWDISTDIR)
	cp -r $(srcdir)/grephyp.c $(HYPVIEWDISTDIR)
	cp -r $(srcdir)/emitpng.c $(HYPVIEWDISTDIR)
	cp -r $(srcdir)/palette.c $(HYPVIEWDISTDIR)
	cp -r $(srcdir)/config.pl $(HYPVIEWDISTDIR)
	cp -r $(srcdir)/hypcache.pl $(HYPVIEWDISTDIR)
	cp -r $(srcdir)/hypviewimg.cgi $(HYPVIEWDISTDIR)
	cp -r $(srcdir)/hypview.cgi $(HYPVIEWDISTDIR)

clean: 
	rm -rf $(DEHYP_OBJS) $(GREPHYP_OBJS)

distclean: clean

