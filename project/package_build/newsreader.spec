Name:           newsreader
Version:        7.0.7
Release:        1%{?dist}
Summary:        Ultra simple desktop feed reader

License:        LGPLv2+
URL:            newsreader-7.0.7.1.fc31.tar.gz
Source0:        https://michaelgautier.github.io/gautier_rss_reader7/release/%{name}-%{version}.%{release}.tar.gz

BuildRequires:  gcc g++ make gtk3-devel libcurl-devel libxml2-devel sqlite-devel webkit2gtk3-devel
Requires:       gtk3 libcurl libxml2 sqlite webkit2gtk3

%description
The newsreader program shows listings from feeds in the RSS/Atom formats. A screen exist to add, remove, or change feed names and urls. 

The program does this in a simple, clean way through a native code UI.

NOTE: Feed providers prefer their site is accessed no earlier than once per hour. The program respects those conditions by default. This is version 7 of a previously privately used binary. Version 7 is the first in the Gautier RSS series to be released in native executable form.

%prep
%setup -c

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
* Mon Jan 20 2020 Michael Gautier
- Clean RPM
