#!/bin/sh
# 
#./configure --prefix=/usr \
#            --sysconfdir=/etc \
#            --mandir=/usr/share/man


echo "RPM build...."
./configure --prefix=$HOME/src/fido-rpm/usr         \
            --sysconfdir=$HOME/src/fido-rpm/etc      \
            --mandir=$HOME/src/fido-rpm/usr/share/man
   
echo "make"
echo "sudo make install"
echo "sudo utils/rpm.sh"

 
