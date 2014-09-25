#!/bin/sh

if ((( $(hostname -s) == "lccns308" )) || (( $(hostname -s) == "lccns178" ))) ; then
  echo "RPM build...."
  ./configure --prefix=$HOME/src/fido-rpm/usr         \
              --sysconfdir=$HOME/src/fido-rpm/etc      \
              --mandir=$HOME/src/fido-rpm/usr/share/man
   
else 
  ./configure --prefix=/usr \
              --sysconfdir=/etc \
              --mandir=/usr/share/man
fi  

