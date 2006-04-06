#!make -f
#
# This software is licensed under the Lesser GNU General Public License.
# Please, see LICENSE.TXT for further information.
#

top_srcdir = .

include $(top_srcdir)/CONFIGVARS

DISTFILE = $(DISTDIR).tar.gz

all:
	cd src; make $@
	cd tools/hypviewcgi; make $@

$(DISTDIR):
	mkdir -p $(DISTDIR)

dist:
	@cd src; make $@
	@cd tools/hypviewcgi; make $@
	tar czf $(DISTDIR)/../libhyp-$(VERSION).tar.gz $(DISTDIR)

distclean: 
	@cd src; make $@
	@cd tools/hypviewcgi; make $@
