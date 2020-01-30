#!/bin/bash
#Primary purpose is to generate a deployment package that validates the program's conformance to Linux standards.

#Goals
#	1) Generate a source package that meets the requirements for Linux distributions.
#	2) Generate a installation package that meets the requirements for Linux distributions.
#	3) Validate program definition in the context of deployment requirements.

#All files included in the source RPM build are to be single licensed under LGPLv2+

#Prepare to build archives, executables and RPMs
## **	Delete stale build files.
rpmdev-wipetree

#RPM Spec file
## **	Establish spec file under the RPM build tree.
cp --remove-destination package_build/newsreader.spec ~/rpmbuild/SPECS/newsreader.spec

#Source Archive - Part 1
## **	Prepare archive setup directories. The goal is to build an archive file used to generate a source package.
rm -rf ../build/rpmbuild/

mkdir -p ../build/rpmbuild/build/
mkdir -p ../build/rpmbuild/source/

#Source Archive - Part 2a
## **	Populate archive setup directories. The goal is to add files to the setup directories pertinent to the installation.
## **	This is the precise source tree needed to build the program.
rsync --human-readable --times --executability --preallocate --update --delay-updates --recursive ../source/rss_ui ../build/rpmbuild/source/
rsync --human-readable --times --executability --preallocate --update --delay-updates --recursive ../source/rss_lib ../build/rpmbuild/source/

rsync --human-readable --times --executability --preallocate --update --delay-updates --recursive ../doc ../build/rpmbuild/
rsync --human-readable --times --executability --preallocate --update --delay-updates --recursive ../desktop ../build/rpmbuild/

#Source Archive - Part 2b
## **	Transform newsreader makefile to the default file name expected by the make program.
rsync --human-readable --times --executability --preallocate --update --delay-updates ./buildscripts/build_newsreader.mk ../build/rpmbuild/build/makefile

#Source Archive - Part 2c
## **	License file distributed with the program.
rsync --human-readable --times --executability --preallocate --update --delay-updates ../LICENSE ../build/rpmbuild/LICENSE

#Source Archive - Part 2d / Alternate origin
## **	Background: https://linuxconfig.org/how-to-create-an-rpm-package
## **	Download the tarball using the spectool.
## **	Command:	spectool -g -R ~/rpmbuild/SPECS/newsreader.spec
## **	You would extract the tarball to ~/somedirectory/rpmbuild/
## **	Afterwards, execute steps at the beginning of this script.

#Source Archive - Part 3
## **	Generate a source tarball that works with the rpmbuild program.
## **	The tarball final tarball will have a name that matches the version tag.
cd ../build/rpmbuild/

tar -czf newsreader.tar.gz *

mv newsreader.tar.gz ~/rpmbuild/SOURCES/v7.0.8.tar.gz

#Generate source RPM
## **	A source RPM will contain instruction on how a package should be configured on a system.
## **	However, success building a source RPM does not mean it can be transformed into an installable RPM.
## **	Consider building an installable RPM (also called a binary RPM) to verify correct definition.
rpmbuild -bs ~/rpmbuild/SPECS/newsreader.spec

#Generate installation RPM (binary executable)
## **	Intent here is to test the generated source RPM while generating the binary RPM.
rpmbuild --rebuild --nodebuginfo ~/rpmbuild/SRPMS/newsreader-7.0.8-1.fc31.src.rpm

#Clean-up
## **	Delete preliminary setup files
rm -rf ../build/rpmbuild/

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
