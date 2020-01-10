Name:		libeasyfc
Version:	0.13.0
Release:	3%{?dist}
Summary:	Easy configuration generator interface for fontconfig

Group:		System Environment/Libraries
License:	LGPLv3+
URL:		http://tagoh.bitbucket.org/libeasyfc/
Source0:	https://bitbucket.org/tagoh/libeasyfc/downloads/%{name}-%{version}.tar.bz2

BuildRequires:	glib2-devel gobject-introspection-devel libxml2-devel fontconfig-devel >= 2.10.92 harfbuzz-devel
BuildRequires:	gettext
Requires:	fontconfig >= 2.10.92

%description
libeasyfc aims to provide an easy interface to generate
fontconfig configuration on demand.

%package	gobject
Summary:	GObject interface for libeasyfc
Group:		System Environment/Libraries
Requires:	%{name}%{?_isa} = %{version}-%{release}

%description	gobject
libeasyfc aims to provide an easy interface to generate
fontconfig configuration on demand.

This package contains an interface for GObject.

%package	devel
Summary:	Development files for libeasyfc
Group:		Development/Libraries
Requires:	%{name}%{?_isa} = %{version}-%{release}
Requires:	pkgconfig
Requires:	fontconfig-devel glib2-devel

%description	devel
libeasyfc aims to provide an easy interface to generate
fontconfig configuration on demand.

This package contains the development files to make any
applications with libeasyfc.

%package	gobject-devel
Summary:	Development files for libeasyfc-gobject
Group:		Development/Libraries
Requires:	%{name}-gobject%{?_isa} = %{version}-%{release}
Requires:	%{name}-devel%{?_isa} = %{version}-%{release}
Requires:	pkgconfig
Requires:	glib2-devel

%description	gobject-devel
libeasyfc aims to provide an easy interface to generate
fontconfig configuration on demand.

This package contains the development files to make any
applications with libeasyfc-gobject.

%prep
%setup -q


%build
%configure --disable-static
make %{?_smp_mflags} V=1


%install
make install DESTDIR=$RPM_BUILD_ROOT INSTALL="/usr/bin/install -p"

rm -f $RPM_BUILD_ROOT%{_libdir}/*.la

%post	-p /sbin/ldconfig
%postun	-p /sbin/ldconfig
%post	gobject -p /sbin/ldconfig
%postun	gobject -p /sbin/ldconfig


%files
%doc README AUTHORS COPYING ChangeLog
%{_libdir}/libeasyfc.so.*

%files	gobject
%{_libdir}/libeasyfc-gobject.so.*
%{_libdir}/girepository-*/Easyfc-*.typelib

%files	devel
%{_includedir}/libeasyfc
%exclude %{_includedir}/libeasyfc/ezfc-gobject.h
%{_libdir}/libeasyfc.so
%{_libdir}/pkgconfig/libeasyfc.pc
%{_datadir}/gtk-doc/html/libeasyfc

%files	gobject-devel
%{_includedir}/libeasyfc/ezfc-gobject.h
%{_libdir}/libeasyfc-gobject.so
%{_libdir}/pkgconfig/libeasyfc-gobject.pc
%{_datadir}/gir-*/Easyfc-*.gir

%changelog
* Fri Jan 24 2014 Daniel Mach <dmach@redhat.com> - 0.13.0-3
- Mass rebuild 2014-01-24

* Fri Dec 27 2013 Daniel Mach <dmach@redhat.com> - 0.13.0-2
- Mass rebuild 2013-12-27

* Tue Jul 30 2013 Akira TAGOH <tagoh@redhat.com> - 0.13.0-1
- New upstream release.

* Fri Mar 29 2013 Akira TAGOH <tagoh@redhat.com> - 0.12.1-1
- New upstream release.

* Mon Feb 25 2013 Akira TAGOH <tagoh@redhat.com> - 0.11-1
- New upstream release.

* Thu Feb 14 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.10-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Tue Dec 18 2012 Akira TAGOH <tagoh@redhat.com> - 0.10-1
- New upstream release.

* Mon Jul 23 2012 Akira TAGOH <tagoh@redhat.com> - 0.9-1
- New upstream release.

* Thu Jul 19 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.8-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Wed Jun 27 2012 Akira TAGOH <tagoh@redhat.com> - 0.8-1
- New upstream release.

* Tue Feb 28 2012 Akira TAGOH <tagoh@redhat.com> - 0.7-1
- New upstream release.

* Wed Feb 08 2012 Akira TAGOH <tagoh@redhat.com> - 0.6-2
- Move .typelib in the libeasyfc-gobject package (#788112)

* Mon Feb 06 2012 Akira TAGOH <tagoh@redhat.com> - 0.6-1
- New upstream release.

* Tue Jan 24 2012 Akira TAGOH <tagoh@redhat.com> - 0.5-1
- New upstream release.

* Wed Jan 18 2012 Akira TAGOH <tagoh@redhat.com> - 0.4-1
- New upstream release.

* Fri Jan 13 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_17_Mass_Rebuild

* Mon Dec 19 2011 Akira TAGOH <tagoh@redhat.com> - 0.3-1
- New upstream release.

* Fri Dec  9 2011 Akira TAGOH <tagoh@redhat.com> - 0.2-1
- New upstream release.
- Removes %%doc from subpackages as per the suggestions in the review.

* Wed Dec  7 2011 Akira TAGOH <tagoh@redhat.com> - 0.1-1
- Initial packaging.

