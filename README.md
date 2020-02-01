RSS Reader by Gautier
=====================

RSS Reader is a way to keep up with your favorite web sites in fast and efficient way. If you regularly visit 2 or more websites every day or every couple of days, RSS is a way to keep up with web sites. That way, you don't have to spend time navigating pages on web sites but have the latest information from the web site come to you.

### RSS = Really Simple Syndication
RSS has been around in prior forms since 1995. Apple had researchers working on a early form of what would eventually become RSS. The official RSS format came out in the year 2000. That means RSS has existed for decades. Wikipedia has <a target="_blank" href="">a moderately detailed article</a> describing what RSS is all about.

#### RSS Logo
Many web sites use the RSS logo (usually at the bottom of their pages) to direct interested users to RSS feeds. On these pages, you can copy the RSS link and paste it into an <a target="_blank" href="https://en.wikipedia.org/wiki/News_aggregator">RSS reader</a> also commonly called a news aggregator.

<img src="https://upload.wikimedia.org/wikipedia/en/4/43/Feed-icon.svg" />

#### RSS Can Be Controversial
RSS has not been without controversy. Aaron Swartz was an activist from MIT. At age 14, he was involved in the creation of RSS as well as markdown. This page uses markdown, a format Aaron helped create. His goal was to make information more accessible.

<b>About: </b><a target="_blank" href="https://en.wikipedia.org/wiki/Aaron_Swartz">Aaron Swartz</a>

#### RSS Feeds Are Widespread
Many, many websites offer RSS. You can use RSS for many things. Many people who know about RSS think it is just for news articles. The reality is RSS can be used for any information source. That can include new publications from sports organizations, scientific and engineering research groups, government statistics and alerts, law enforcement, business meetings, and literally any data that can be described by the RSS data format.

You can go on a search engine like Google, Duck Duck Go, or Bing and search for RSS feeds. An example would be: <b><i>Washington Post RSS</i></b> which may bring back search results to a page where you can copy RSS web addresses and paste them into an RSS reader. Notice I don't use the term news aggregator since RSS can be used by web sites to publish information other than news.

#### A Good RSS Reader
The problem is not so much in encoding data. The issue with RSS is <i>reading it</i> productively. Although there are dozens of news aggregators in existence, I think the world can use one that may prove to be a steady, long-term processor of RSS data. Some of the best ones from the early 2000s were bought out and then summarily retired or only worked on Microsoft Windows. Many of them disappeared never to be seen again. 

A highly acclaimed RSS reader called TinyTinyRSS is a good concept but way out of reach for most people. What is needed is a simple to use, easy to install RSS reader that will remain free and clean to use. I tried super simple RSS readers that you run from the command-line to generate an HTML page you can read. I have zero issues with using the command-line but needed something more GUI oriented. I sought to produce an RSS reader that is more practical to use on a daily basis.

<img src="https://gautiertalkstechnology.files.wordpress.com/2020/01/rss-reader-2020-01-01a.png?w=1024&h=563" />

The goal I have with the RSS reader is to ensure that a simple RSS reader exist for Linux. I tried some RSS readers on Linux and some of them started out neat and simple but the ones I liked eventually grew into a social media gateway. That was more than I wanted. I think it is wise to have and use tools that allow you to access information in a straightforward way without any strings attached.

#### Video Demo
The following video shows how the program works in the real-world. When the program is compiled, this video shows how the program actually works.
##### YouTube Video - RSS Reader v7

<a href="http://www.youtube.com/watch?feature=player_embedded&v=ylKjd1bKu7k
" target="_blank"><img src="http://img.youtube.com/vi/ylKjd1bKu7k/0.jpg" 
alt="RSS Reader by Gautier v7" width="240" height="180" border="10" /></a>

#### Background Information
The RSS reader discussed here is still under development. It is production level but I have other features and refinements underway. When I say production level, you can use it just as described in the video. I use it myself for day-to-day use. That is the only way to truly certify software.

I discuss the RSS reader at a general level on one of my blogs.

<b>Blog link: </b><a target="_blank" href="https://gautiertalkstechnology.wordpress.com/2020/01/01/rss-version-7-video/">https://gautiertalkstechnology.wordpress.com/2020/01/01/rss-version-7-video/</a>

That link also shows the video above and puts on <i>in proximity</i> to other articles about the RSS reader program. Numerous articles on that blog present details about the RSS reader. One of those articles triggered a WordPress shutdown on 1/2/2020 which is why this page now exist.

#### Installing RSS Reader by Gautier (Compile from Source)
The program can be compiled from source using the makefile in __[root directory]__/project/buildscripts/build_newsreader.mk under make. The way that works is you copy that file to the ***[root directory]***/build/ directory. In that directory, run this command:
    make --file=build_newsreader.mk

Running that command in a Linux environment will produce the file, __[root directory]__/build/bin/__newsreader__. You can copy the newsreader program file to another directory and run it from there. The newsreader program can then be ran either by double-clicking it from a desktop file manager or in the Terminal with __./newsreader__.

Compiling it from source however means you have the following libraries and tools installed:
- Development libraries and headers for gtk, sqlite, curl, webkitgtk, and libxml
- g++, make

#### Installing RSS Reader by Gautier (Build an installer)
An install file for Fedora and Ubuntu can be made in the form of a .rpm or .deb file. The way that is done is to setup a virtual machine running either Fedora or Ubuntu. You will need to setup the build server to build packages. The scripts under __[root directory]__/project/package_build/ contains the means to coordinate the creation of an install file. Further details can be found at the following page: <a target="_blank" href="https://gautiertalkstechnology.wordpress.com/2020/01/31/gautier-rss-linux-deployment-packages-rpm-and-deb/">Gautier RSS – Linux Deployment Packages (RPM and DEB)</a>.

#### Learn Software Development
The RSS reader program is also a way to learn software development using C++, the language the program is written in. I published a <b><a target="_blank" href="https://gautiertalkstechnology.wordpress.com/2019/11/01/cpp-the-basic-way-ui-and-command-line-22-videos/">22 video series about software development in C++</a></b> that would provide the requisite knowledge to build and compile the program from scratch. If you follow the guide, you can create the program manually under Linux. The videos can provide a jumpstart to learning and using C++ to create UIs in Linux.
