#!make -f
#
# This software is licensed under the Lesser GNU General Public License.
# Please, see LICENSE.TXT for further information.
#

top_srcdir = ../..
srcdir     = .

include $(top_srcdir)/CONFIGVARS

TARGET = dehyp grephyp

all: $(TARGET)

CFLAGS = -I$(top_srcdir)/include $(CROSS_CFLAGS)

# engine binary objects
DEHYP_OBJS = emitpng.o dehyp.o
GREPHYP_OBJS = grephyp.o

dehyp.o: $(top_srcdir)/include/libhyp.h
grephyp.o: $(top_srcdir)/include/libhyp.h

dehyp: $(DEHYP_OBJS) $(libhyp_dir)/libhyp.a
	$(CC) -o $@ $(DEHYP_OBJS) -L$(libhyp_dir) -lhyp -L$(libpng_dir) -lpng $(CROSS_LDFLAGS)

grephyp: $(GREPHYP_OBJS) $(libhyp_dir)/libhyp.a
	$(CC) -o $@ $(GREPHYP_OBJS) -L$(libhyp_dir) -lhyp $(CROSS_LDFLAGS)

strip:
	$(STRIP) --strip-all $(TARGET)

clean: 
	rm -rf $(DEHYP_OBJS) $(GREPHYP_OBJS)
