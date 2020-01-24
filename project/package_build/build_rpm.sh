#!/bin/bash
#All files included in the source RPM build are to be single licensed under LGPLv2+

rpmdev-wipetree

cp --remove-destination package_build/newsreader.spec ~/rpmbuild/SPECS/newsreader.spec

rm -rf ../build/rpmbuild/

mkdir -p ../build/rpmbuild/build/
mkdir -p ../build/rpmbuild/source/

rsync --human-readable --times --executability --preallocate --update --delay-updates --recursive ../source/rss_ui ../build/rpmbuild/source/
rsync --human-readable --times --executability --preallocate --update --delay-updates --recursive ../source/rss_lib ../build/rpmbuild/source/
rsync --human-readable --times --executability --preallocate --update --delay-updates --recursive ../source/style ../build/rpmbuild/source/

rsync --human-readable --times --executability --preallocate --update --delay-updates ./buildscripts/build_newsreader.mk ../build/rpmbuild/build/makefile
rsync --human-readable --times --executability --preallocate --update --delay-updates ../LICENSE ../build/rpmbuild/LICENSE

cd ../build/rpmbuild/

tar -czf newsreader.tar.gz *

mv newsreader.tar.gz ~/rpmbuild/SOURCES/v7.0.7.tar.gz

rpmbuild -bs ~/rpmbuild/SPECS/newsreader.spec
rpmbuild --rebuild --nodebuginfo ~/rpmbuild/SRPMS/newsreader-7.0.7-1.fc31.src.rpm

rm -rf ../build/rpmbuild/
