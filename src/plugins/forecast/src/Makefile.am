if DEBUG
DBGFLAGS = -g -DDEBUG
endif

AM_CXXFLAGS = -Wall -Wno-unused -O3 -fPIC -std=c++11 -msse2 -fpermissive \
              -I$(top_srcdir)/src/opaq_core \
	       $(HDF5_CPPFLAGS) \
              -I$(top_srcdir)/src/depend/TinyXML-2.6.2 -DTIXML_USE_STL \
              -I$(top_srcdir)/src/depend/gzstream-1.5 \
              -I$(LOG4CXX_INCLUDE) \
              -I$(EIGEN_PATH)


AM_LDFLAGS = -avoid-version -module -shared -export-dynamic

# the plugins
pkglib_LTLIBRARIES = FeedForwardModel.la

FeedForwardModel_la_SOURCES = FeedForwardModel.cpp FeedForwardModel.h \
                              Tune.cpp Tune.h FeatureGenerator.h \
                              scaler.cpp scaler.h layer.cpp layer.h \
                              feedforwardnet.cpp feedforwardnet.h transfcn.h \
                              OVL_model_1_7CST.h \
                              OVL_model_2_7CST.h \
                              OVL_model_3_7CST.h

FeedForwardModel_la_LIBADD   = $(top_srcdir)/src/opaq_core/libopaq.la
