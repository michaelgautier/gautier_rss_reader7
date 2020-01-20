# =========================================================================
#     This makefile was generated by
#     Bakefile 0.2.11 (http://www.bakefile.org)
#     Do not modify, all changes will be overwritten!
# =========================================================================



# -------------------------------------------------------------------------
# These are configurable options:
# -------------------------------------------------------------------------

# 'install' program location 
INSTALL ?= install

# Location where the package is installed by 'make install' 
prefix ?= /usr/local

# Destination root (/ is used if empty) 
DESTDIR ?= 

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
NEWSREADER_CXXFLAGS = -std=c++17 -O3 -flto `pkg-config gtk+-3.0 --cflags` \
	`pkg-config webkit2gtk-4.0 --cflags` `xml2-config --cflags` -pedantic-errors \
	-w -Waddress -Waddress-of-packed-member -Waggregate-return -Waligned-new \
	-Wall -Walloca -Walloc-zero -Warray-bounds -Wbool-compare -Wbool-operation \
	-Wcast-align -Wcast-function-type -Wcast-qual -Wcatch-value \
	-Wchar-subscripts -Wclass-memaccess -Wclobbered -Wcomment \
	-Wconditionally-supported -Wconversion -Wconversion-null \
	-Wcoverage-mismatch -Wctor-dtor-privacy -Wdangling-else -Wdate-time \
	-Wdelete-incomplete -Wdelete-non-virtual-dtor -Wdeprecated-copy \
	-Wdeprecated-copy-dtor -Wdisabled-optimization -Wdouble-promotion \
	-Wduplicated-branches -Wduplicated-cond -Weffc++ -Wempty-body -Wenum-compare \
	-Wexpansion-to-defined -Wextra -Wfatal-errors -Wfloat-conversion \
	-Wfloat-equal -Wformat -Wformat-nonliteral -Wformat-security \
	-Wformat-signedness -Wformat-y2k -Wframe-address -Whsa -Wif-not-aligned \
	-Wignored-attributes -Wignored-qualifiers -Wimplicit-fallthrough \
	-Winit-self -Winline -Wint-in-bool-context -Winvalid-memory-model \
	-Winvalid-pch -Wliteral-suffix -Wlogical-not-parentheses -Wlogical-op \
	-Wlong-long -Wmain -Wmaybe-uninitialized -Wmemset-elt-size \
	-Wmemset-transposed-args -Wmisleading-indentation -Wmissing-attributes \
	-Wmissing-braces -Wmissing-field-initializers -Wmissing-format-attribute \
	-Wmissing-include-dirs -Wmissing-noreturn -Wmissing-profile \
	-Wmultiple-inheritance -Wmultistatement-macros -Wnamespaces -Wnarrowing \
	-Wno-aggressive-loop-optimizations -Wno-attributes -Wno-attribute-warning \
	-Wno-builtin-declaration-mismatch -Wno-builtin-macro-redefined \
	-Wno-class-conversion -Wno-cpp -Wno-deprecated -Wno-deprecated-declarations \
	-Wno-div-by-zero -Wno-endif-labels -Wnoexcept -Wno-format-contains-nul \
	-Wno-format-extra-args -Wno-free-nonheap-object -Wno-init-list-lifetime \
	-Wno-int-to-pointer-cast -Wno-invalid-offsetof -Wno-multichar -Wnonnull \
	-Wnonnull-compare -Wno-non-template-friend -Wnon-virtual-dtor -Wno-overflow \
	-Wno-pedantic-ms-format -Wno-pmf-conversions -Wno-pragmas \
	-Wno-prio-ctor-dtor -Wno-return-local-addr -Wno-scalar-storage-order \
	-Wno-terminate -Wno-unused-result -Wnull-dereference -Wodr -Wold-style-cast \
	-Wopenmp-simd -Woverlength-strings -Woverloaded-virtual -Wpacked \
	-Wpacked-bitfield-compat -Wpacked-not-aligned -Wpadded -Wparentheses \
	-Wpedantic -Wpessimizing-move -Wplacement-new -Wpointer-arith \
	-Wpointer-compare -Wredundant-decls -Wredundant-move -Wregister -Wreorder \
	-Wrestrict -Wreturn-type -Wsequence-point -Wshadow -Wshift-count-negative \
	-Wshift-count-overflow -Wshift-negative-value -Wshift-overflow \
	-Wsign-compare -Wsign-conversion -Wsign-promo -Wsizeof-array-argument \
	-Wsizeof-pointer-div -Wsizeof-pointer-memaccess -Wstack-protector \
	-Wstrict-aliasing -Wstrict-null-sentinel -Wstrict-overflow \
	-Wstringop-truncation -Wsubobject-linkage -Wsuggest-final-methods \
	-Wsuggest-final-types -Wsuggest-override -Wswitch -Wswitch-bool \
	-Wswitch-default -Wswitch-enum -Wswitch-unreachable -Wsync-nand \
	-Wsystem-headers -Wtautological-compare -Wtemplates -Wtrampolines \
	-Wtrigraphs -Wtype-limits -Wundef -Wuninitialized -Wunknown-pragmas -Wunused \
	-Wunused-but-set-parameter -Wunused-but-set-variable \
	-Wunused-const-variable -Wunused-function -Wunused-label \
	-Wunused-local-typedefs -Wunused-macros -Wunused-parameter -Wunused-value \
	-Wunused-variable -Wuseless-cast -Wvariadic-macros \
	-Wvector-operation-performance -Wvirtual-inheritance -Wvla \
	-Wvolatile-register-var -Wwrite-strings -Wzero-as-null-pointer-constant \
	-I../source $(CPPFLAGS) $(CXXFLAGS)
NEWSREADER_OBJECTS =  \
	newsreader_application.o \
	newsreader_app_win.o \
	newsreader_article_frame.o \
	newsreader_article_header.o \
	newsreader_headlines_frame.o \
	newsreader_rss_manage.o \
	newsreader_rss_article.o \
	newsreader_rss_feed.o \
	newsreader_rss_reader.o \
	newsreader_rss_util.o \
	newsreader_rss_writer.o \
	newsreader_db.o \
	newsreader_feed_download.o \
	newsreader_feed_parse.o

### Conditionally set variables: ###



### Targets: ###

all: bin/newsreader

install: install_newsreader

uninstall: uninstall_newsreader

clean: 
	rm -f ./*.o
	rm -f ./*.d
	rm -f bin/newsreader

bin: 
	@mkdir -p bin

bin/newsreader: $(NEWSREADER_OBJECTS) bin
	$(CXX) -o $@ $(NEWSREADER_OBJECTS)  -std=c++17 -no-pie -lpthread `pkg-config gtk+-3.0 --libs` `pkg-config sqlite3 --libs` `pkg-config libcurl --libs` `pkg-config webkit2gtk-4.0 --libs` `xml2-config --libs` $(LDFLAGS)

install_newsreader: bin/newsreader
	$(INSTALL) -d $(DESTDIR)$(prefix)/bin
	install -c bin/newsreader $(DESTDIR)$(prefix)/bin

uninstall_newsreader: 
	rm -f $(DESTDIR)$(prefix)/bin/newsreader

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

newsreader_rss_manage.o: ./../source/rss_ui/rss_manage/impl/rss_manage.cpp
	$(CXX) -c -o $@ $(NEWSREADER_CXXFLAGS) $(CPPDEPS) $<

newsreader_rss_article.o: ./../source/rss_lib/rss/impl/rss_article.cpp
	$(CXX) -c -o $@ $(NEWSREADER_CXXFLAGS) $(CPPDEPS) $<

newsreader_rss_feed.o: ./../source/rss_lib/rss/impl/rss_feed.cpp
	$(CXX) -c -o $@ $(NEWSREADER_CXXFLAGS) $(CPPDEPS) $<

newsreader_rss_reader.o: ./../source/rss_lib/rss/impl/rss_reader.cpp
	$(CXX) -c -o $@ $(NEWSREADER_CXXFLAGS) $(CPPDEPS) $<

newsreader_rss_util.o: ./../source/rss_lib/rss/impl/rss_util.cpp
	$(CXX) -c -o $@ $(NEWSREADER_CXXFLAGS) $(CPPDEPS) $<

newsreader_rss_writer.o: ./../source/rss_lib/rss/impl/rss_writer.cpp
	$(CXX) -c -o $@ $(NEWSREADER_CXXFLAGS) $(CPPDEPS) $<

newsreader_db.o: ./../source/rss_lib/db/impl/db.cpp
	$(CXX) -c -o $@ $(NEWSREADER_CXXFLAGS) $(CPPDEPS) $<

newsreader_feed_download.o: ./../source/rss_lib/rss_download/impl/feed_download.cpp
	$(CXX) -c -o $@ $(NEWSREADER_CXXFLAGS) $(CPPDEPS) $<

newsreader_feed_parse.o: ./../source/rss_lib/rss_parse/impl/feed_parse.cpp
	$(CXX) -c -o $@ $(NEWSREADER_CXXFLAGS) $(CPPDEPS) $<

.PHONY: all install uninstall clean install_newsreader uninstall_newsreader


# Dependencies tracking:
-include ./*.d
