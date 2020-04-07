#!/bin/bash
#Primary purpose is to generate a deployment package that validates the program's conformance to Linux standards.
#This is based on the build_rpm.sh script since that script came first.

#Goals
#	1) Generate a source package that meets the requirements for Linux distributions.
#	2) Generate a installation package that meets the requirements for Linux distributions.
#	3) Validate program definition in the context of deployment requirements.

#All files included in the source DEB build are to be single licensed under LGPLv2+

#Prepare to build archives, executables and RPMs
## **	Delete stale build files.
mkdir -p ~/debbuild/
rm -rf ~/debbuild/
mkdir -p ~/debbuild/newsreader-7.7/

mkdir -p ~/debbuild_tmp/
rm -rf ~/debbuild_tmp/
mkdir -p ~/debbuild_tmp/

echo "-------------------------------------------------------------"
pwd
ls -gGh
#DEB Package Configuration
## **	Establish build configuration under the debian/ build tree.
rsync --human-readable --times --executability --preallocate --update --delay-updates --recursive ../project/package_build/debian ~/debbuild/newsreader-7.7

#Source Archive - Part 1
## **	Populate archive setup directories. The goal is to add files to the setup directories pertinent to the installation.
## **	This is the precise source tree needed to build the program.
rsync --human-readable --times --executability --preallocate --update --delay-updates --recursive ../source/rss_ui ~/debbuild_tmp/source/
rsync --human-readable --times --executability --preallocate --update --delay-updates --recursive ../source/rss_lib ~/debbuild_tmp/source/

rsync --human-readable --times --executability --preallocate --update --delay-updates --recursive ../doc ~/debbuild_tmp/
rsync --human-readable --times --executability --preallocate --update --delay-updates --recursive ../desktop ~/debbuild_tmp/

#Source Archive - Part 2a
## **	Transform newsreader makefile to the default file name expected by the make program.
cp --update --preserve ./buildscripts/build_newsreader.mk ~/debbuild_tmp/makefile
cp --update --preserve ./buildscripts/build_newsreader.mk ~/debbuild/newsreader-7.7/makefile

#Source Archive - Part 3
## **	Generate a source tarball that works with the debian/ubuntu build program of choice.
## **	Nearly all debian / ubuntu build processes use a source archive (compressed zip or tar.gz file).
## **	The tarball final tarball will have a name that matches the version tag.
echo "-------------------------------------------------------------"
cd ~/debbuild_tmp/

cp --recursive --preserve * ~/debbuild/

tar -czf newsreader.tar.gz *

cp --preserve newsreader.tar.gz ~/debbuild/newsreader_7.7.0.orig.tar.gz

pwd
ls -gGh ~/debbuild_tmp/

echo "-------------------------------------------------------------"
cd ~/debbuild/
pwd
ls -gGh ~/debbuild/

#Generate source DEB
## **	A source DEB will contain instruction on how a package should be configured on a system.
## **	However, success building a source DEB does not mean it can be transformed into an installable DEB.
## **	Consider building an installable DEB (also called a binary DEB) to verify correct definition.
cd ~/debbuild/newsreader-7.7/
debuild -us -uc

echo "-------------------------------------------------------------"
pwd
ls -gGh ~/debbuild/newsreader-7.7/

cd ~/debbuild/
echo "-------------------------------------------------------------"
pwd
ls -gGh ~/debbuild/

#Clean-up
## **	Delete preliminary setup files
rm -rf ~/debbuild_tmp/

