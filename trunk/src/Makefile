#!make -f
#
# This software is licensed under the Lesser GNU General Public License.
# Please, see LICENSE.TXT for further information.
#

top_srcdir = ..

include $(top_srcdir)/CONFIGVARS

LIBHYP = libhyp.a
VERSION= 0.5.0

TARGETDIR = $(top_srcdir)/lib
TARGET = $(TARGETDIR)/$(LIBHYP)
DISTDIR = $(TARGETDIR)/libhyp-$(VERSION)

INCDIR = $(top_srcdir)/include

all: $(TARGET)

#STDLIB_CFLAGS = $(M68K_ATARI_MINT_CFLAGS)

CFLAGS  = -I$(INCDIR) $(STDLIB_CFLAGS)

CSRC = \
	decodlh5.c \
	list.c \
	hypobj.c

# engine binary objects
OBJECTS = $(CSRC:.c=.o)

hypobj.o: $(INCDIR)/libhyp.h hypobj.h

$(TARGET): $(OBJECTS) $(TARGETDIR)
	$(AR) cr $@ $(OBJECTS); $(RANLIB) $@

$(TARGETDIR):
	mkdir -p $(TARGETDIR)

dist:
	mkdir -p $(DISTDIR)/include
	cp $(top_srcdir)/include/libhyp.h $(DISTDIR)/include
	mkdir -p $(DISTDIR)/src
	cp $(top_srcdir)/src/Makefile $(DISTDIR)/src
	cp $(top_srcdir)/src/*.c $(DISTDIR)/src
	cp $(top_srcdir)/src/*.h $(DISTDIR)/src
	cp $(top_srcdir)/CONFIGVARS $(DISTDIR)
	cp $(top_srcdir)/COPYRIGHT $(DISTDIR)

strip:
	$(STRIP) --strip-all $(TARGET)

clean: 
	rm -rf $(OBJECTS)
