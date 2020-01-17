Name:           newsreader
Version:        7.0.7
Release:        1%{?dist}
Summary:        Ultra simple desktop feed reader

License:        LGPLv2+
URL:            https://michaelgautier.github.io/gautier_rss_reader7/
Source0:        https://michaelgautier.github.io/gautier_rss_reader7/release/%{name}-%{version}.%{release}.tar.gz

BuildRequires:  gcc
BuildRequires:  g++
BuildRequires:  make
BuildRequires:  gtk3-devel
BuildRequires:  libcurl-devel
BuildRequires:  libxml2-devel
BuildRequires:  sqlite-devel
BuildRequires:  webkit2gtk3-devel

Requires:        gtk3-devel
Requires:        libcurl-devel
Requires:        libxml2-devel
Requires:        sqlite-devel
Requires:        webkit2gtk3-devel     

%description
The newsreader program shows listings from feeds in the RSS/Atom formats. A screen exist to add, remove, or change feed names and urls. 

The program does this in a simple, clean way through a native code UI.

NOTE: Feed providers prefer their site is accessed no earlier than once per hour. The program respects those conditions by default. This is version 7 of a previously privately used binary. Version 7 is the first in the Gautier RSS series to be released in native executable form.

%prep
cp %{_sourcedir}/%{name}-%{version}.%{release}.tar.gz %{_builddir}
tar -xf %{name}-%{version}.%{release}.tar.gz

%build
cd %{_builddir}/build/
make %{?_smp_mflags} 

%install
mkdir -p %{buildroot}/%{_bindir}
install -m 0755 %{_builddir}/build/bin/%{name} %{buildroot}/%{_bindir}/%{name}

%files
%license LICENSE
%{_bindir}/%{name}

%changelog
* Fri Jan 17 2020 Michael Gautier
- RPM package built with tito
