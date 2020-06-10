%define    dist .el6
Summary:   Fido - a file monitoring utility
Name:      fido
Version:   1.1.6
Release:   1%{?dist}
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
/etc/init.d/fido
/etc/sysconfig/fido
/usr/share/man/man1/fido.1.gz
/usr/share/man/man5/fido.conf.5.gz
/usr/sbin/fido

%post
if [ -f /etc/fido.bak ] ; then
  rm -Rf /etc/fido 
  mv /etc/fido.bak /etc/fido
fi

%changelog
* Wed May 27 2020 Jeffrey Fulmer <jeff@joedog.org>
- Updated for 1.1.6
* Fri Sep 26 2014 Jeffrey Fulmer <jeff@joedog.org>
- Fixed versioning
* Fri Apr  5 2013 Jeffrey Fulmer <jeff@joedog.org>
- Added logic to preserve existing /etc/fido
* Mon Mar  4 2013 Jeffrey Fulmer <jeff@joedog.org>
- Updated for release 1.0.8
* Mon Mar 31 2008 Jeffrey Fulmer <jeff@joedog.org>
- Created initial spec file
