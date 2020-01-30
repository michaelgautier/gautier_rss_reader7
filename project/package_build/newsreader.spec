Name:           newsreader
Version:        7.0.8
Release:        1%{?dist}
Summary:        Ultra simple desktop feed reader

#All files included in the source RPM build are to be single licensed under LGPLv2+
License:        LGPLv2+
URL:            https://michaelgautier.github.io/gautier_rss_reader7/
Source0:        https://github.com/michaelgautier/gautier_rss_reader7/archive/v%{version}.tar.gz

BuildRequires:  gcc-c++ make pkgconfig(gthread-2.0) pkgconfig(gtk+-3.0) pkgconfig(sqlite3) pkgconfig(libcurl) pkgconfig(webkit2gtk-4.0) pkgconfig(libxml-2.0)
BuildRequires:  libappstream-glib
Requires:       gtk3 sqlite webkit2gtk3

%description
Shows Internet feeds in a clean and obvious way. Comes with a screen that helps  
you add new feeds or purge those you no longer need. 

The program uses a native code UI that runs smooth with no bloat.

NOTE:

- Feed providers prefer their site is accessed no earlier than once per hour.
- The program respects those conditions by default.

- This version 7 has no public predecessors with an installation package.
- Previous versions existed on the Internet in source code form.
- Earlier versions exist that were not packaged for installation.
- Version 7 is the first Gautier RSS program packaged for installation.

%global _hardened_build 1

#Extract archive
%prep
%setup -cq

#Compile / Link
%build
cd %{_builddir}/%{name}-%{version}/build/

make %{?_smp_mflags} 

#Configure
%install
#The file relocations below (/usr/local/bin to /usr/bin) are due to policy requirements in rpmlint
#https://unix.stackexchange.com/questions/125120/why-is-dir-or-file-in-usr-local-an-error-rather-than-a-warning?rq=1
#I decided to keep the default policies in place an override their indications in the spec file.
rm -rf %{buildroot}

mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_datadir}/%{name}/
mkdir -p %{buildroot}%{_datadir}/applications/
mkdir -p %{buildroot}%{_metainfodir}
mkdir -p %{buildroot}%{_mandir}/man7/

cd %{_builddir}/%{name}-%{version}/build/

#Default output to */usr/local/bin
#See https://refspecs.linuxfoundation.org/FHS_3.0/fhs/ch04s09.html
%{make_install}

#Migrate from staged build in */usr/local/bin to /usr/bin
cp --remove-destination --preserve %{buildroot}/usr/local/bin/%{name} %{buildroot}%{_bindir}/%{name}
strip --strip-all %{buildroot}%{_bindir}/%{name}

#Set dependencies under */usr/share/
cp --remove-destination --preserve %{buildroot}/usr/local/share/applications/%{name}.desktop %{buildroot}%{_datadir}/applications/%{name}.desktop
cp --remove-destination --preserve %{buildroot}/usr/local/share/metainfo/%{name}.appdata.xml %{buildroot}%{_metainfodir}/%{name}.appdata.xml
cp --remove-destination --preserve %{buildroot}/usr/local/share/man/man7/%{name}.7* %{buildroot}%{_mandir}/man7/

desktop-file-install --dir=%{buildroot}%{_datadir}/applications/ %{buildroot}%{_datadir}/applications/%{name}.desktop
desktop-file-validate %{buildroot}%{_datadir}/applications/%{name}.desktop
appstream-util validate-relax --nonet %{buildroot}%{_metainfodir}/%{name}.appdata.xml

rm %{buildroot}/usr/local/bin/%{name}
rm %{buildroot}/usr/local/share/applications/%{name}.desktop
rm %{buildroot}/usr/local/share/metainfo/%{name}.appdata.xml
rm %{buildroot}/usr/local/share/man/man7/%{name}.7*

#Remove build
%clean
rm -rf %{buildroot}

#Enumerate files
%files
%license LICENSE
%{_bindir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_metainfodir}/%{name}.appdata.xml
%{_mandir}/man7/%{name}.7*

%changelog
* Sat Jan 25 2020 Michael Gautier <michaelgautier.wordpress.com> - 7.0.8-1
- Clean RPM
