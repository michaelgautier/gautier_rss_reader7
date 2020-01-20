#!/bin/bash

rpmdev-wipetree

cp --remove-destination package_build/newsreader.spec ~/rpmbuild/SPECS/newsreader.spec

rm -rf ../build/rpmbuild/

mkdir -p ../build/rpmbuild/build/

rsync --human-readable --times --executability --preallocate --update --recursive --delay-updates ../source ../build/rpmbuild
rsync --human-readable --times --executability --preallocate --update --delay-updates ./buildscripts/build_newsreader.mk ../build/rpmbuild/build/makefile
rsync --human-readable --times --executability --preallocate --update --delay-updates ../LICENSE ../build/rpmbuild/LICENSE

cd ../build/rpmbuild/

tar -czf newsreader.tar.gz *

mv newsreader.tar.gz ~/rpmbuild/SOURCES/newsreader-7.0.7.1.fc31.tar.gz

rpmbuild -bs ~/rpmbuild/SPECS/newsreader.spec
rpmbuild --rebuild --nodebuginfo ~/rpmbuild/SRPMS/newsreader-7.0.7-1.fc31.src.rpm

rm -rf ../build/rpmbuild/
