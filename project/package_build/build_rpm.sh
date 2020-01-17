#!/bin/bash

rpmdev-wipetree

cp package_build/newsreader.spec ~/rpmbuild/SPECS/

rm -rf ../build/rpmbuild/

mkdir ../build/
mkdir ../build/rpmbuild/
mkdir ../build/rpmbuild/build/

rsync --human-readable --times --executability --preallocate --update --recursive --delay-updates ../source ../build/rpmbuild
rsync --human-readable --times --executability --preallocate --update --delay-updates ./buildscripts/build_newsreader.mk ../build/rpmbuild/build/makefile
rsync --human-readable --times --executability --preallocate --update --delay-updates ../LICENSE ../build/rpmbuild/LICENSE

cd ../build/rpmbuild/

tar -czf newsreader-7.0.7.1.fc31.tar.gz *

mv newsreader-7.0.7.1.fc31.tar.gz ~/rpmbuild/SOURCES/

rpmbuild -bs ~/rpmbuild/SPECS/newsreader.spec
rpmbuild --rebuild ~/rpmbuild/SRPMS/newsreader-7.0.7-1.fc31.src.rpm

rm -rf ../build/rpmbuild/
