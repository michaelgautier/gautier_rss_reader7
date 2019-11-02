#!/bin/bash

bakefile --output=../build/build_newsreader.mk --format=gnu --verbose bakefiles/prog_newsreader.bkl
bakefile --output=../build/build_newsdownld.mk --format=gnu --verbose bakefiles/prog_newsdownld.bkl
bakefile --output=../build/build_rss_io_test.mk --format=gnu --verbose bakefiles/prog_rss_io_test.bkl

