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
rm -rf $RPM_BUILD_ROOT
cd %{_builddir}/%{name}-%{version}/build/

#RPM macro %{make_install} will place the final result in ~/rpmbuild/BUILDROOT/%{name}-%{version}-%{release}.%{_arch}
#Again %{buildroot} usually equals ~/rpmbuild/BUILDROOT/%{name}-%{version}-%{release}.%{_arch}

#That is great for keeping the package from actually installing on your system if you are just packaging.
#Also great if you are creating the package and need to rebuild the package several times before it is finalized.
#See https://unix.stackexchange.com/questions/180666/rpm-install-section

#The problem is that the package will go to %{buildroot}/usr/local/bin
#/usr/local/bin should be /usr/bin and will cause issues.
#RPM macro %{make_install} sets the value of DESTDIR correctly
#However, the value supplied to /usr/bin/install is $DESTDIR/usr/local/bin instead of $DESTDIR/usr/bin
#That will cause a conflict if the value of %{_bindir} does not match the value expected by rpmlint which is /usr/bin.

%{make_install}

#%{make_install} on Fedora 31 server expands out to:
#	/usr/bin/make install DESTDIR=/home/yourname/rpmbuild/BUILDROOT/newsreader-7.0.7-1.fc31.x86_64 'INSTALL=/usr/bin/install -p' PREFIX=/usr
#	/usr/bin/install -p -d /home/yourname/rpmbuild/BUILDROOT/newsreader-7.0.7-1.fc31.x86_64/usr/local/bin
#	install -c bin/newsreader /home/yourname/rpmbuild/BUILDROOT/newsreader-7.0.7-1.fc31.x86_64/usr/local/bin

#since the default value of %{make_install} does not match the default expectation of rpmlint (%_bindir), this might be a bug that will require a permanent workaround.

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
