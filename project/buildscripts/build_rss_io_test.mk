# =========================================================================
#     This makefile was generated by
#     Bakefile 0.2.11 (http://www.bakefile.org)
#     Do not modify, all changes will be overwritten!
# =========================================================================



# -------------------------------------------------------------------------
# These are configurable options:
# -------------------------------------------------------------------------

#  
AR ?= ar

#  
RANLIB ?= ranlib

# C compiler 
CC = gcc

# C++ compiler 
CXX = g++

# Standard flags for CC 
CFLAGS ?= 

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
ARGTABLE_CFLAGS =   -c -std=c17 -O3 $(CPPFLAGS) $(CFLAGS)
ARGTABLE_OBJECTS =  \
	argtable_argtable3.o
RSS_IO_TEST_CXXFLAGS = -std=c++17 -pipe -O2 -ggdb -D_FORTIFY_SOURCE=2 \
	-fasynchronous-unwind-tables -fcf-protection=full \
	-fdelete-null-pointer-checks -feliminate-unused-debug-symbols -fexceptions \
	-fno-common -fnon-call-exceptions -fsized-deallocation -fsplit-stack \
	-fstack-clash-protection -fstack-protector-all -fstrict-aliasing -ftrapv \
	-fvisibility=hidden -Werror -Wfatal-errors -pedantic-errors -Wall -Walloca \
	-Walloc-zero -Wcast-align=strict -Wcast-function-type -Wcast-qual \
	-Wcatch-value=3 -Wconversion -Wdate-time -Wdisabled-optimization \
	-Wdouble-promotion -Wduplicated-branches -Wduplicated-cond -Wextra \
	-Wextra-semi -Wfloat-equal -Wformat=2 -Wformat-overflow=2 \
	-Wformat-signedness -Wformat-truncation=2 -Wlogical-op -Wno-unused-function \
	-Wno-maybe-uninitialized -Wmissing-profile -Wparentheses -Wpedantic \
	-Wplacement-new=2 -Wredundant-decls -Wno-shadow -Wno-sizeof-pointer-div \
	-Wstack-protector -Wstrict-overflow=5 -Wstringop-overflow=4 \
	-Wstringop-truncation -Wsuggest-final-methods -Wsuggest-final-types \
	-Wsuggest-override -Wtrampolines -Wundef -Wuninitialized -Wunknown-pragmas \
	-Wunused-const-variable=2 -Wunused-macros -Wunused-parameter \
	-Wzero-as-null-pointer-constant `xml2-config --cflags` -I../source \
	$(CPPFLAGS) $(CXXFLAGS)
RSS_IO_TEST_OBJECTS =  \
	rss_io_test_rss_io_test.o \
	rss_io_test_db.o \
	rss_io_test_feed_download.o \
	rss_io_test_rss_article.o \
	rss_io_test_rss_feed.o \
	rss_io_test_rss_reader.o \
	rss_io_test_rss_util.o \
	rss_io_test_rss_writer.o \
	rss_io_test_feed_parse.o

### Conditionally set variables: ###



### Targets: ###

all: bin/libargtable.a bin/rss_io_test

install: 

uninstall: 

clean: 
	rm -f ./*.o
	rm -f ./*.d
	rm -f bin/libargtable.a
	rm -f bin/rss_io_test

bin: 
	@mkdir -p bin

bin/libargtable.a: $(ARGTABLE_OBJECTS) bin
	rm -f $@
	$(AR) rcu $@ $(ARGTABLE_OBJECTS)
	$(RANLIB) $@

bin/rss_io_test: $(RSS_IO_TEST_OBJECTS) bin bin/libargtable.a
	$(CXX) -o $@ $(RSS_IO_TEST_OBJECTS)  -std=c++17 -pipe -O2 -flto -ggdb -flinker-output=pie argtable_argtable3.o `pkg-config sqlite3 --libs` `pkg-config libcurl --libs` `xml2-config --libs` $(LDFLAGS)

argtable_argtable3.o: ./../source/external/argtable/argtable3.c
	$(CC) -c -o $@ $(ARGTABLE_CFLAGS) $(CPPDEPS) $<

rss_io_test_rss_io_test.o: ./../source/rss_cli/rss_io_test.cpp
	$(CXX) -c -o $@ $(RSS_IO_TEST_CXXFLAGS) $(CPPDEPS) $<

rss_io_test_db.o: ./../source/rss_lib/db/impl/db.cpp
	$(CXX) -c -o $@ $(RSS_IO_TEST_CXXFLAGS) $(CPPDEPS) $<

rss_io_test_feed_download.o: ./../source/rss_lib/rss_download/impl/feed_download.cpp
	$(CXX) -c -o $@ $(RSS_IO_TEST_CXXFLAGS) $(CPPDEPS) $<

rss_io_test_rss_article.o: ./../source/rss_lib/rss/impl/rss_article.cpp
	$(CXX) -c -o $@ $(RSS_IO_TEST_CXXFLAGS) $(CPPDEPS) $<

rss_io_test_rss_feed.o: ./../source/rss_lib/rss/impl/rss_feed.cpp
	$(CXX) -c -o $@ $(RSS_IO_TEST_CXXFLAGS) $(CPPDEPS) $<

rss_io_test_rss_reader.o: ./../source/rss_lib/rss/impl/rss_reader.cpp
	$(CXX) -c -o $@ $(RSS_IO_TEST_CXXFLAGS) $(CPPDEPS) $<

rss_io_test_rss_util.o: ./../source/rss_lib/rss/impl/rss_util.cpp
	$(CXX) -c -o $@ $(RSS_IO_TEST_CXXFLAGS) $(CPPDEPS) $<

rss_io_test_rss_writer.o: ./../source/rss_lib/rss/impl/rss_writer.cpp
	$(CXX) -c -o $@ $(RSS_IO_TEST_CXXFLAGS) $(CPPDEPS) $<

rss_io_test_feed_parse.o: ./../source/rss_lib/rss_parse/impl/feed_parse.cpp
	$(CXX) -c -o $@ $(RSS_IO_TEST_CXXFLAGS) $(CPPDEPS) $<

.PHONY: all install uninstall clean


# Dependencies tracking:
-include ./*.d
