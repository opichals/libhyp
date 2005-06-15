#!make -f
#
# This software is licensed under the Lesser GNU General Public License.
# Please, see LICENSE.TXT for further information.
#

top_srcdir = ..

include $(top_srcdir)/CONFIGVARS

LIBHYP  = libhyp.a
VERSION = `cat $(top_srcdir)/src/.makeversion`


TARGETDIR = $(top_srcdir)/lib
TARGET = $(TARGETDIR)/$(LIBHYP)
DISTDIR = $(TARGETDIR)/libhyp-$(VERSION)

INCDIR = $(top_srcdir)/include

all: $(TARGET)

CFLAGS = -I$(INCDIR) $(CROSS_CFLAGS)

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

.makeversion: $(INCDIR)/libhyp.h
	echo -e '#include <libhyp.h>\n MAKE_LIBHYP_VERSION __LIBHYP_VERSION_MAJOR__ . __LIBHYP_VERSION_MINOR__ . __LIBHYP_VERSION_REVISION__' | $(CC) $(CFLAGS) -E - | grep MAKE_LIBHYP_VERSION | sed -e "s/MAKE_LIBHYP_VERSION //g" | sed -e "s/ //g" > $@


dist:	.makeversion
	@make dist-ver

dist-copy:
	mkdir -p $(DISTDIR)/include
	cp $(top_srcdir)/include/libhyp.h $(DISTDIR)/include
	mkdir -p $(DISTDIR)/src
	cp $(top_srcdir)/src/Makefile $(DISTDIR)/src
	cp $(top_srcdir)/src/*.c $(DISTDIR)/src
	cp $(top_srcdir)/src/*.h $(DISTDIR)/src
	cp $(top_srcdir)/CONFIGVARS $(DISTDIR)
	cp $(top_srcdir)/COPYRIGHT $(DISTDIR)

dist-ver: dist-copy
	tar czf $(DISTDIR)/../libhyp-$(VERSION).tar.gz $(DISTDIR)


strip:
	$(STRIP) --strip-all $(TARGET)

clean: 
	rm -rf $(OBJECTS)
