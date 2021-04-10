#!/bin/sh
# 
CMD=$1

echo $CMD | grep -q -i "rpm" 
if [ $? -eq 0 ] 
then
  echo "RPM build...."
  ./configure --prefix=$HOME/src/fido-rpm/usr         \
              --sysconfdir=$HOME/src/fido-rpm/etc      \
              --mandir=$HOME/src/fido-rpm/usr/share/man
else 
  ./configure --prefix=/usr \
              --sysconfdir=/etc \
              --mandir=/usr/share/man
fi
   
echo "make"
echo "sudo make install"
echo "sudo utils/rpm.sh"

 
