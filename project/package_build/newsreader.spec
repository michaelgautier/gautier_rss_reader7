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
%make_install
cp %{_builddir}/%{name}-%{version}/source/style/app.css %{buildroot}/usr/local/bin/app.css

%files
%license LICENSE
/usr/local/bin/%{name}
/usr/local/bin/app.css

%changelog
* Fri Jan 24 2020 Michael Gautier <michaelgautier.wordpress.com> - 7.0.7-1
- Clean RPM
