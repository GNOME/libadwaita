%global _vpath_srcdir %{name}

Name:    libhandy
Version: 0.90.0
Release: 1%{?dist}
Summary: A library full of GTK widgets for mobile phones

License: LGPLv2+
Url:     https://gitlab.gnome.org/GNOME/libhandy
Source0: https://gitlab.gnome.org/GNOME/libhandy/archive/master.tar.gz

BuildRequires: gcc
BuildRequires: gobject-introspection
BuildRequires: gtk-doc
BuildRequires: meson >= 0.40.1
BuildRequires: pkgconfig(gio-2.0)
BuildRequires: pkgconfig(gladeui-2.0)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(gmodule-2.0)
BuildRequires: pkgconfig(gtk+-3.0)
BuildRequires: pkgconf-pkg-config
BuildRequires: vala

%description
%{summary}.

%package devel
Summary: Development libraries, headers, and documentation for %{name}
Requires: libhandy = %{version}-%{release}

%description devel
%{summary}.

%prep
%setup -c -q

%build
%meson -Dexamples=false -Dgtk_doc=true
%meson_build

%install
%meson_install

%files
%{_libdir}/libhandy*.so.*
%{_libdir}/girepository-1.0/Handy*.typelib

%files devel
%{_includedir}/libhandy*
%{_libdir}/libhandy*.so
%{_libdir}/pkgconfig/libhandy*.pc
%{_datadir}/gir-1.0/Handy*.gir
%{_datadir}/glade/catalogs/libhandy.xml
%{_datadir}/vala/vapi/libhandy*.deps
%{_datadir}/vala/vapi/libhandy*.vapi
%{_datadir}/gtk-doc

%changelog
* Fri May 18 2018 Julian Richen <julian@richen.io> - 0.0.0-1
- Update to 0.0.0-1
