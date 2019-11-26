#!/bin/bash

#Consider this a test script. Takes RSS feeds that were already downloaded as XML files and updates the rss database.
#Creates the RSS database if it does not exist.

../build/bin/newsdownld --xmldb --rss-feedname="Ars Technica" --rss-url="http://feeds.arstechnica.com/arstechnica/index"
../build/bin/newsdownld --xmldb --rss-feedname="Slashdot" --rss-url="http://rss.slashdot.org/Slashdot/slashdotMain"
../build/bin/newsdownld --xmldb --rss-feedname="Phoronix" --rss-url="https://www.phoronix.com/rss.php"
../build/bin/newsdownld --xmldb --rss-feedname="TechCrunch" --rss-url="http://feeds.feedburner.com/TechCrunch/"
../build/bin/newsdownld --xmldb --rss-feedname="BBC" --rss-url="http://feeds.bbci.co.uk/news/rss.xml?edition=int"
../build/bin/newsdownld --xmldb --rss-feedname="The Atlantic" --rss-url="https://www.theatlantic.com/feed/best-of/"
../build/bin/newsdownld --xmldb --rss-feedname="DistroWatch" --rss-url="https://distrowatch.com/news/dww.xml"
../build/bin/newsdownld --xmldb --rss-feedname="Linux Kernel" --rss-url="https://www.kernel.org/feeds/all.atom.xml"
../build/bin/newsdownld --xmldb --rss-feedname="Tech Rights" --rss-url="http://techrights.org/feed/"

