AM_CXXFLAGS = -Wall -Wno-unused -O3 -fPIC -std=c++11 -msse2 -fpermissive \
              -I$(top_srcdir)/src/opaq_core \
               $(HDF5_CPPFLAGS) \
              -I$(top_srcdir)/src/depend/TinyXML-2.6.2 -DTIXML_USE_STL \
              -I$(top_srcdir)/src/depend/gzstream-1.5 \
              -I$(LOG4CXX_INCLUDE) \
              -I$(EIGEN_PATH)\
              -I../../src

AM_LDFLAGS  = -L$(top_srcdir)/src/depend/gzstream-1.5 -lgzstream \
              -L$(top_srcdir)/src/depend/TinyXML-2.6.2 -ltinyxml \
               $(LOG4CXX) $(HDF5_LDFLAGS) $(HDF5_LIBS)

bin_PROGRAMS = opaq_ffsim
#opaq_ffsim_LDADD = 
opaq_ffsim_SOURCES = ffsim.cpp ../../src/scaler.cpp ../../src/layer.cpp ../../src/feedforwardnet.cpp
