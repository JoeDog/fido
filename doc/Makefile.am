##
## doc/Makefile.am
##
## Copyright (C) 2011 by
## Jeffrey Fulmer - <jeff@joedog.org>, et al.
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
##

AUTOMAKE_OPTIONS  =    foreign no-dependencies 

man_MANS          =    fido.1 fido.conf.5

EXTRA_DIST        =    fido.pod fido.1.in fido.conf.5.in fido.conf.pod apache.conf jboss.conf fido.conf.sample

DISTCLEANFILES    =    $(man_MANS) fido.1.in fido.conf.5.in

FIDOCONF          =    $(sysconfdir)/fido.conf.sample

RULESDIR          =    $(sysconfdir)/rules

install-exec-hook:
	@if ! test -d $(DESTDIR)$(sysconfdir); then \
	  $(mkinstalldirs) $(DESTDIR)$(sysconfdir); \
	fi; \
	if ! test -d $(DESTDIR)$(RULESDIR); then \
	  $(mkinstalldirs) $(DESTDIR)$(RULESDIR); \
	fi;
	$(INSTALL_DATA) $(srcdir)/fido.conf.sample $(DESTDIR)$(FIDOCONF)

