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
Shows Internet feeds in a clean and obvious way. Comes with a screen that helps  
you add new feeds or purge those you no longer need. 

The program uses a native code UI that runs smooth with no bloat.

NOTE:

- Feed providers prefer their site is accessed no earlier than once per hour.
- The program respects those conditions by default.

- This version 7 has no public predecessors as an installable program.
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
mkdir -p %{buildroot}%{_datadir}/%{name}
cd %{_builddir}/%{name}-%{version}/build/

#Default output to */usr/local/bin
#See https://refspecs.linuxfoundation.org/FHS_3.0/fhs/ch04s09.html
%{make_install}

#Migrate from staged build in */usr/local/bin to /usr/bin
cp --remove-destination --preserve %{buildroot}/usr/local/bin/%{name} %{buildroot}%{_bindir}/%{name}

#Set dependencies under */usr/share/%{name}
cp --remove-destination --preserve %{_builddir}/%{name}-%{version}/source/style/app.css %{buildroot}%{_datadir}/%{name}/app.css

rm %{buildroot}/usr/local/bin/%{name}

#Remove build
%clean
rm -rf %{buildroot}

#Enumerate files
%files
%license LICENSE
%{_bindir}/%{name}
%{_datadir}/%{name}/app.css

#For later...

#/usr/bin/%{name}
#/usr/share/applications/%{name}.desktop
#/usr/share/doc/%{name}/ChangeLog
#/usr/share/doc/%{name}/README.md
#/usr/share/%{name}/styles/app.css
#/usr/share/%{name}/images/%{name}.png
#/usr/share/%{name}/images/%{name}.svg
#/usr/share/icons/hicolor/48x48/apps/%{name}.png
#/usr/share/icons/hicolor/scalable/apps/%{name}.svg
#/usr/share/man/man1/%{name}.7.gz

%changelog
* Fri Jan 24 2020 Michael Gautier <michaelgautier.wordpress.com> - 7.0.7-1
- Clean RPM
