#!make -f
#
# This software is licensed under the Lesser GNU General Public License.
# Please, see LICENSE.TXT for further information.
#

top_srcdir = ../..
srcdir     = .

include $(top_srcdir)/CONFIGVARS

libhyp_dir = $(top_srcdir)/lib

TARGET = dehyp

all: $(TARGET)

CFLAGS = -I$(top_srcdir)/include $(CROSS_CFLAGS)

CSRC = \
	emitpng.c \
	dehyp.c

# engine binary objects
OBJECTS = $(CSRC:.c=.o)

dehyp.o: $(top_srcdir)/include/libhyp.h

$(TARGET): $(OBJECTS) $(libhyp_dir)/libhyp.a
	$(CC) -o $(TARGET) $(OBJECTS) -L$(libhyp_dir) -lhyp -lpng $(CROSS_LDFLAGS)

strip:
	$(STRIP) --strip-all $(TARGET)

clean: 
	rm -rf $(OBJECTS)
