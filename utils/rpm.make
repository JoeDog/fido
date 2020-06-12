PACKAGE=fido
VERSION=1.1.6
SPECFILE=fido.spec
GZIP=/bin/gzip
TAR=/bin/tar
RPMBUILD=/usr/bin/rpmbuild
SRCDIR=/usr/src/redhat/SOURCES
ZIP=$(PACKAGE)-$(VERSION).zip
TOP=/usr/src/redhat
SRC=$(TOP)/SOURCES/fido-$(VERSION).tar.gz
SUDO=/usr/bin/sudo
DIRS=etc usr


all: dist build

dist:
	$(TAR) cvf - $(DIRS) | $(GZIP) -f > $(PACKAGE)-$(VERSION).tar.gz
	$(SUDO) mv $(PACKAGE)-$(VERSION).tar.gz $(SRCDIR)

build:
	$(SUDO) $(RPMBUILD) -ba $(SPECFILE)

clean:
	rm $(TOP)/SOURCES/$(PACKAGE)-$(VERSION).tar.gz
	rm -Rf $(TOP)/BUILD/$(PACKAGE)-$(VERSION)
	rm $(TOP)/SRPMS/$(PACKAGE)-$(VERSION)*
	rm $(TOP)/RPMS/x86_64/$(PACKAGE)-debuginfo*

