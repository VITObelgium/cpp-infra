# tiny xml makefile

EXTRA_DIST = 	changes.txt \
		readme.txt \
		utf8test.gif \
		utf8test.xml \
		utf8testverify.xml


TESTS = xmltest

check_PROGRAMS =	xmltest
xmltest_SOURCES =	xmltest.cpp
xmltest_LDADD = 	libtinyxml.la
xmltest_CXXFLAGS =	-DTIXML_USE_STL

noinst_LTLIBRARIES =		libtinyxml.la
libtinyxml_la_SOURCES = 	tinyxml.cpp tinyxml.h \
				tinyxmlparser.cpp \
				tinyxmlerror.cpp \
				tinystr.cpp tinystr.h
libtinyxml_la_CXXFLAGS =	-DTIXML_USE_STL

DISTCLEANFILES = utf8testout.xml \
		 demotest.xml    \
		 textfile.txt    \
		 test5.xml       \
		 test6.xml       \
		 test7.xml
