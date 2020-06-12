%define    dist .el7
%define    _topdir /usr/src/redhat
Summary:   Fido - a file monitoring utility
Name:      fido
Version:   1.1.6
Release:   2%{?dist}
Buildarch: x86_64
Group:     Monitoring
License:   GPL
Source:    ftp://ftp.joedog.org/pub/fido/fido-%{version}.tar.gz
BuildRoot: /var/tmp/%{name}-buildroot
AutoReq:   0


%description
A file monitoring utility

%define __find_requires /bin/true

%prep
%setup -c -q

%pre
if [ -f /etc/fido ] ; then
  cp -r /etc/fido /etc/fido.bak
fi

%install
cp --verbose -a ./ $RPM_BUILD_ROOT/

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)

/etc/fido
/etc/fido/fido.conf.sample
/etc/fido/rules
/usr/share/man/man1/fido.1.gz
/usr/share/man/man5/fido.conf.5.gz
/usr/sbin/fido
/usr/lib/systemd/system/fido.service

%post
if [ -f /etc/fido.bak ] ; then
  rm -Rf /etc/fido 
  mv /etc/fido.bak /etc/fido
fi
systemctl enable fido.service

%changelog
* Wed Jun 10 2020 Jeffrey Fulmer <jeff@joedog.org>
- Created initial RHEL7 spec file
