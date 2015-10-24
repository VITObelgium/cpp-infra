EXTRA_DIST = COPYING.LIB\
             README\
             index.html\
             version\
             logo.gif 

AM_CXXFLAGS = -I. -O3 -fPIC
AM_LDFLAGS  = -lz

libgzstream_la_SOURCES = gzstream.C gzstream.h
noinst_LTLIBRARIES = libgzstream.la

check_PROGRAMS = test_gzip test_gunzip
test_gzip_SOURCES = test_gzip.C
test_gzip_LDADD = libgzstream.la

test_gunzip_SOURCES = test_gunzip.C
test_gunzip_LDADD = libgzstream.la
