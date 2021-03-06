#!/bin/bash

bakefile --output=buildscripts/build_newsreader.mk --format=gnu --verbose bakefiles/prog_newsreader.bkl
bakefile --output=buildscripts/build_newsreader_clang.mk --format=gnu --verbose bakefiles/prog_newsreader_clang.bkl
bakefile --output=buildscripts/build_newsreader_mingw.mk --format=gnu --verbose bakefiles/prog_newsreader_mingw.bkl

bakefile --output=buildscripts/build_newsdownld.mk --format=gnu --verbose bakefiles/prog_newsdownld.bkl
bakefile --output=buildscripts/build_rss_io_test.mk --format=gnu --verbose bakefiles/prog_rss_io_test.bkl

