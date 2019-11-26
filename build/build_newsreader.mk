# =========================================================================
#     This makefile was generated by
#     Bakefile 0.2.11 (http://www.bakefile.org)
#     Do not modify, all changes will be overwritten!
# =========================================================================



# -------------------------------------------------------------------------
# These are configurable options:
# -------------------------------------------------------------------------

# C++ compiler 
CXX = g++

# Standard flags for C++ 
CXXFLAGS ?= 

# Standard preprocessor flags (common for CC and CXX) 
CPPFLAGS ?= 

# Standard linker flags 
LDFLAGS ?= 



# -------------------------------------------------------------------------
# Do not modify the rest of this file!
# -------------------------------------------------------------------------

### Variables: ###

CPPDEPS = -MT$@ -MF`echo $@ | sed -e 's,\.o$$,.d,'` -MD -MP
NEWSREADER_CXXFLAGS = -std=c++17 -Og -ggdb `pkg-config gtk+-3.0 --cflags` \
	`xml2-config --cflags` -I../source $(CPPFLAGS) $(CXXFLAGS)
NEWSREADER_OBJECTS =  \
	newsreader_application.o \
	newsreader_app_win.o \
	newsreader_article_frame.o \
	newsreader_article_header.o \
	newsreader_headlines_frame.o \
	newsreader_rss_article.o \
	newsreader_reader.o \
	newsreader_writer.o \
	newsreader_db.o \
	newsreader_feed_download.o \
	newsreader_feed_parse.o

### Conditionally set variables: ###



### Targets: ###

all: bin/newsreader

install: 

uninstall: 

clean: 
	rm -f ./*.o
	rm -f ./*.d
	rm -f bin/newsreader

bin: 
	@mkdir -p bin

bin/newsreader: $(NEWSREADER_OBJECTS) bin
	$(CXX) -o $@ $(NEWSREADER_OBJECTS)  -std=c++17 `pkg-config gtk+-3.0 --libs` `pkg-config sqlite3 --libs` `pkg-config libcurl --libs` `xml2-config --libs` $(LDFLAGS)

newsreader_application.o: ./../source/rss_ui/impl/application.cpp
	$(CXX) -c -o $@ $(NEWSREADER_CXXFLAGS) $(CPPDEPS) $<

newsreader_app_win.o: ./../source/rss_ui/app_win/impl/app_win.cpp
	$(CXX) -c -o $@ $(NEWSREADER_CXXFLAGS) $(CPPDEPS) $<

newsreader_article_frame.o: ./../source/rss_ui/app_win/impl/article_frame.cpp
	$(CXX) -c -o $@ $(NEWSREADER_CXXFLAGS) $(CPPDEPS) $<

newsreader_article_header.o: ./../source/rss_ui/app_win/impl/article_header.cpp
	$(CXX) -c -o $@ $(NEWSREADER_CXXFLAGS) $(CPPDEPS) $<

newsreader_headlines_frame.o: ./../source/rss_ui/app_win/impl/headlines_frame.cpp
	$(CXX) -c -o $@ $(NEWSREADER_CXXFLAGS) $(CPPDEPS) $<

newsreader_rss_article.o: ./../source/rss_lib/rss/impl/rss_article.cpp
	$(CXX) -c -o $@ $(NEWSREADER_CXXFLAGS) $(CPPDEPS) $<

newsreader_reader.o: ./../source/rss_lib/rss/impl/reader.cpp
	$(CXX) -c -o $@ $(NEWSREADER_CXXFLAGS) $(CPPDEPS) $<

newsreader_writer.o: ./../source/rss_lib/rss/impl/writer.cpp
	$(CXX) -c -o $@ $(NEWSREADER_CXXFLAGS) $(CPPDEPS) $<

newsreader_db.o: ./../source/rss_lib/db/impl/db.cpp
	$(CXX) -c -o $@ $(NEWSREADER_CXXFLAGS) $(CPPDEPS) $<

newsreader_feed_download.o: ./../source/rss_lib/rss_download/impl/feed_download.cpp
	$(CXX) -c -o $@ $(NEWSREADER_CXXFLAGS) $(CPPDEPS) $<

newsreader_feed_parse.o: ./../source/rss_lib/rss_parse/impl/feed_parse.cpp
	$(CXX) -c -o $@ $(NEWSREADER_CXXFLAGS) $(CPPDEPS) $<

.PHONY: all install uninstall clean


# Dependencies tracking:
-include ./*.d
