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

# The general forecast model sources, used by the different plugins 
MPL_FeedForwardSources = MLP_FeedForwardModel.cpp \
                         MLP_FeedForwardModel.h \
                         Tune.cpp \
                         Tune.h \
                         scaler.cpp \
                         scaler.h \
                         layer.cpp \
                         layer.h \
                         feedforwardnet.cpp \
                         feedforwardnet.h \
                         transfcn.h

# The effective plugins, based upon the general abstract classes
pkglib_LTLIBRARIES = OVL_IRCEL_model1.la\
                     OVL_IRCEL_model2.la

OVL_IRCEL_model1_la_SOURCES = $(MPL_FeedForwardSources) OVL_IRCEL_model1.cpp OVL_IRCEL_model1.h
OVL_IRCEL_model1_la_LIBADD  = $(top_srcdir)/src/opaq_core/libopaq.la
                            
OVL_IRCEL_model2_la_SOURCES = $(MPL_FeedForwardSources) OVL_IRCEL_model2.cpp OVL_IRCEL_model2.h
OVL_IRCEL_model2_la_LIBADD  = $(top_srcdir)/src/opaq_core/libopaq.la
