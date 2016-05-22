##
## src/Makefile.am
##
## Copyright (C) 2008 by
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

sbin_PROGRAMS          =   fido

WARN_CFLAGS            =   @WARN_CFLAGS@

AM_CFLAGS              =   $(WARN_CFLAGS) $(PTHREAD_CFLAGS)

LDFLAGS                =   $(PTHREAD_LDFLAGS)

fido_SOURCES           =    \
array.c    array.h          \
base16.c   base16.h         \
conf.c     conf.h           \
crew.c     crew.h           \
date.c     date.h           \
ereg.c     ereg.h           \
ereg2.h    setup.h          \
fido.c     fido.h           \
getopt.c   getopt1.c        \
hash.c     hash.h           \
logger.c   logger.h         \
pid.c      pid.h            \
main.c     ansidecl.h       \
memory.c   memory.h         \
notify.c   notify.h         \
perl.c     perl.h           \
rset.c     rset.h           \
rule.c     rule.h           \
runner.c   runner.h         \
throttle.c throttle.h       \
util.c     util.h           \
version.c  version.h

AUTOMAKE_OPTIONS       =   foreign no-dependencies                   

