#!/bin/sh

PREFIX=$1
SYSTEMD=false
RELEASE=$(uname -r | sed 's/^.*el\([0-9]\+\).*$/\1/')

if [ ! -e "$1" ] ; then
  echo "Usage: rpm.sh /path/to/rmpbuild/dir"
  exit 1
fi

if [ ! -d "${PREFIX}" ]; then
  echo "${PREFIX} does not exist"
  exit 1
fi

if [ -e ./fido-redhat-start ] ; then
  BASE="."
elif [ -e utils/fido-redhat-start ]; then
  BASE="utils"
else
  echo "Can't find the startup script"
  echo "Did you run this from the top-level source directory?"
  echo
fi


if [ -d "/run/systemd/system" ]; then
  SYSTEMD=true
fi

if [ $SYSTEMD == true ]; then
  echo "SYSTEMD"
  if [ ! -e "${PREFIX}/usr/lib/systemd/system" ] ; then
    mkdir -p "${PREFIX}/usr/lib/systemd/system"
    chown root:root "${PREFIX}/usr"
    chown root:root "${PREFIX}/usr/lib"
    chown root:root "${PREFIX}/usr/lib/systemd"
    chown root:root "${PREFIX}/usr/lib/systemd/system"
  fi
  cp "${BASE}/fido.service" "${PREFIX}/usr/lib/systemd/system"
  chmod 644 "${PREFIX}/usr/lib/systemd/system/fido.service"
  cp "${BASE}/rpm-${RELEASE}.spec" "${PREFIX}/fido.spec"
  cp "${BASE}/rpm.make"   "${PREFIX}/Makefile"
else
  ## INSTALL /etc/init.d/fido
  ##         /etc/sysconfig/fido 
  ##         NOTE: handle chkconfig
  echo "SYSVINIT system" 
  if [ ! -e "${PREFIX}/etc/init.d" ] ; then
    mkdir -p "${PREFIX}/etc/init.d"
    chown root:root "${PREFIX}/etc"
    chown root:root "${PREFIX}/etc/init.d"
  fi 
  if [ ! -e "${PREFIX}/etc/sysconfig" ] ; then
    mkdir -p "${PREFIX}/etc/sysconfig"
    chown root:root "${PREFIX}/etc/sysconfig"
  fi
  cp "${BASE}/fido-redhat-start" "${PREFIX}/etc/init.d/fido"
  chown root:root "${PREFIX}/etc/init.d/fido" 
  chmod 775 "${PREFIX}/etc/init.d/fido" 
  cp  "${BASE}/fido-redhat-config" "${PREFIX}/etc/sysconfig/fido"
  chown root:root "${PREFIX}/etc/sysconfig/fido"
  chmod 644 "${PREFIX}/etc/sysconfig/fido"
  cp "${BASE}/rpm-${RELEASE}.spec" "${PREFIX}/fido.spec"
  cp "${BASE}/rpm.make"   "${PREFIX}/Makefile"
fi

