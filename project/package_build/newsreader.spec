Name:           newsreader
Version:        7.0.7
Release:        1%{?dist}
Summary:        Ultra simple desktop feed reader

#All files included in the source RPM build are to be single licensed under LGPLv2+
License:        LGPLv2+
URL:            https://michaelgautier.github.io/gautier_rss_reader7/
Source0:        https://github.com/michaelgautier/gautier_rss_reader7/archive/v%{version}.tar.gz

BuildRequires:  gcc-c++ make pkgconfig(gthread-2.0) pkgconfig(gtk+-3.0) pkgconfig(sqlite3) pkgconfig(libcurl) pkgconfig(webkit2gtk-4.0) pkgconfig(libxml-2.0)
Requires:       gtk3 sqlite webkit2gtk3

%description
The newsreader program shows listings from feeds in the RSS/Atom formats. 
A screen exist to add, remove, or change a feed's name and URL. 

The program does this in a simple, clean way through a native code UI.

NOTE: 
Feed providers prefer their site is accessed no earlier than once per hour.
The program respects those conditions by default.
This is version 7 of a previously privately used binary.
Version 7 is the first Gautier RSS program packaged for installation.

%global _hardened_build 1

%prep
%setup -cq

%build
cd %{_builddir}/%{name}-%{version}/build/
make %{?_smp_mflags} 

%install
cd %{_builddir}/%{name}-%{version}/build/
%{make_install}

mkdir -p %{buildroot}/usr/bin/
cp --remove-destination --preserve %{buildroot}/usr/local/bin/newsreader %{buildroot}/usr/bin/newsreader
cp --remove-destination --preserve %{_builddir}/%{name}-%{version}/source/style/app.css %{buildroot}/usr/bin/app.css
rm %{buildroot}/usr/local/bin/newsreader

%files
%license LICENSE
%{_bindir}/%{name}
%{_bindir}/app.css

#final spec file needs to resemble the example at:
#	https://linuxconfig.org/how-to-create-an-rpm-package

#/usr/bin/newsreader
#/usr/share/applications/newsreader.desktop
#/usr/share/doc/newsreader/ChangeLog
#/usr/share/doc/newsreader/README.md
#/usr/share/newsreader/styles/app.css
#/usr/share/newsreader/images/newsreader.png
#/usr/share/newsreader/images/newsreader.svg
#/usr/share/icons/hicolor/48x48/apps/newsreader.png
#/usr/share/icons/hicolor/scalable/apps/newsreader.svg
#/usr/share/man/man1/newsreader.1.gz

%changelog
* Fri Jan 24 2020 Michael Gautier <michaelgautier.wordpress.com> - 7.0.7-1
- Clean RPM
