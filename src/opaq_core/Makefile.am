if DEBUG
# enable debugging symbols and no optimization
AM_CXXFLAGS = -g -O0 -DDEBUG -Wall -Wno-unused -fPIC -fpermissive -std=c++11 $(HDF5_CPPFLAGS) \
           -I$(top_srcdir)/src/depend/TinyXML-2.6.2 -DTIXML_USE_STL \
           -I$(top_srcdir)/src/depend/gzstream-1.5 \
           -I$(LOG4CXX_INCLUDE)
else
#include optimization
AM_CXXFLAGS = -O3 -fPIC -fpermissive -std=c++11 $(HDF5_CPPFLAGS) \
           -I$(top_srcdir)/src/depend/TinyXML-2.6.2 -DTIXML_USE_STL \
           -I$(top_srcdir)/src/depend/gzstream-1.5 \
           -I$(LOG4CXX_INCLUDE)
endif

AM_LDFLAGS  = -L$(top_srcdir)/src/depend/gzstream-1.5 -lgzstream \
              -L$(top_srcdir)/src/depend/TinyXML-2.6.2 -ltinyxml \
              $(LOG4CXX) $(HDF5_LDFLAGS) $(HDF5_LIBS)


libopaq_la_LDFLAGS = $(AM_LDFLAGS) -release $(VERSION)

lib_LTLIBRARIES = libopaq.la

libopaq_la_SOURCES =\
AQNetwork.cpp\
AQNetworkProvider.cpp\
Cell.cpp\
Component.cpp\
ComponentManager.cpp\
ConfigurationHandler.cpp\
DateTime.cpp\
Engine.cpp\
ForecastHorizon.cpp\
Grid.cpp\
Model.cpp\
Point.cpp\
Pollutant.cpp\
PollutantManager.cpp\
Station.cpp\
TimeInterval.cpp\
config/Component.cpp\
config/OpaqRun.cpp\
config/Plugin.cpp\
config/ForecastStage.cpp\
config/MappingStage.cpp\
data/DataBuffer.cpp\
data/DataProvider.cpp\
data/DataStore.cpp\
data/GridProvider.cpp\
data/ForecastOutputWriter.cpp\
tools/AQNetworkTools.cpp\
tools/DateTimeTools.cpp\
tools/FileTools.cpp\
tools/GzipReader.cpp\
tools/Hdf5Tools.cpp\
tools/StringTools.cpp\
tools/XmlTools.cpp


otherincludedir = $(includedir)/opaq
# use nobase_ prefix since we want to keep these header files
nobase_otherinclude_HEADERS = $(shell find . -type f -name '*.h')

# see: http://www.gnu.org/savannah-checkouts/gnu/automake/manual/html_node/Sources.html
BUILT_SOURCES = opaq.h
opaq.h:
	HEADERS=$(shell find . -type f -name *.h)
	echo '#ifndef OPAQ_H_' > opaq.h
	echo '#define OPAQ_H_' >> opaq.h
	/bin/bash -c 'for header in $(HEADERS); do echo "#include \"$$header\"" >> opaq.h; done'
	echo '#endif' >> opaq.h

# the opaq binary
bin_PROGRAMS = opaq
opaq_LDADD = libopaq.la
opaq_SOURCES = opaq.cpp

install-exec-hook:
	-ldconfig

