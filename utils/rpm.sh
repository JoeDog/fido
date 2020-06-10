#!/bin/sh

PREFIX=$1
SYSTEMD=false

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
  cp "${BASE}/rpm-6.spec" "${PREFIX}/fido.spec"
  cp "${BASE}/rpm.make"   "${PREFIX}/Makefile"
fi

