#!/bin/bash

glib-compile-resources --sourcedir=../source/rss_ui/ --generate-source ../source/rss_ui/app_resources.xml
glib-compile-resources --sourcedir=../source/rss_ui/ --generate-header ../source/rss_ui/app_resources.xml
glib-compile-resources --sourcedir=../source/rss_ui/ --generate-dependencies ../source/rss_ui/app_resources.xml

