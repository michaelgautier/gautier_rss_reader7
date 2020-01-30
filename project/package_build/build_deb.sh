#!/bin/bash
#Primary purpose is to generate a deployment package that validates the program's conformance to Linux standards.

#Goals
#	1) Generate a source package that meets the requirements for Linux distributions.
#	2) Generate a installation package that meets the requirements for Linux distributions.
#	3) Validate program definition in the context of deployment requirements.

#All files included in the source DEB build are to be single licensed under LGPLv2+

#Prepare to build archives, executables and DEBs
## **	Delete stale build files.
mkdir -p ~/newsreader-package/
rm -rf ~/newsreader-package/
mkdir -p ~/newsreader-package/

mkdir -p ~/newsreader-package.orig/
rm -rf ~/newsreader-package.orig/
mkdir -p ~/newsreader-package.orig/build/

cp --recursive --preserve ../project/package_build/debian/ ~/newsreader-package/

rsync --human-readable --times --executability --preallocate --update --delay-updates --recursive ../source/rss_ui ~/newsreader-package.orig/source/
rsync --human-readable --times --executability --preallocate --update --delay-updates --recursive ../source/rss_lib ~/newsreader-package.orig/source/

rsync --human-readable --times --executability --preallocate --update --delay-updates --recursive ../doc ~/newsreader-package.orig/
rsync --human-readable --times --executability --preallocate --update --delay-updates --recursive ../desktop ~/newsreader-package.orig/

## **	Transform newsreader makefile to the default file name expected by the make program.
rsync --human-readable --times --executability --preallocate --update --delay-updates ./buildscripts/build_newsreader.mk ~/newsreader-package.orig/build/makefile

cd ~/newsreader-package/
debuild -us -uc


