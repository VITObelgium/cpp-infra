if DEBUG
AM_CXXFLAGS = -g -O0 -DDEBUG -Wall -Wno-unused -fPIC -std=c++11 -fpermissive \
              -I$(top_srcdir)/src/opaq_core $(HDF5_CPPFLAGS) \
              -I$(top_srcdir)/src/depend/TinyXML-2.6.2 -DTIXML_USE_STL \
              -I$(top_srcdir)/src/depend/gzstream-1.5 \
              -I$(LOG4CXX_INCLUDE)
else
AM_CXXFLAGS = -fPIC -std=c++11 -fpermissive \
              -I$(top_srcdir)/src/opaq_core $(HDF5_CPPFLAGS) \
              -I$(top_srcdir)/src/depend/TinyXML-2.6.2 -DTIXML_USE_STL \
              -I$(top_srcdir)/src/depend/gzstream-1.5 \
              -I$(LOG4CXX_INCLUDE)
endif

AM_LDFLAGS = -avoid-version -module -shared -export-dynamic

# the plugins
pkglib_LTLIBRARIES = IRCELMeteoProvider.la \
		     Hdf5Buffer.la \
		     RioObsProvider.la \
		     XMLAQNetProvider.la \
		     XmlGridProvider.la \
		     AsciiForecastWriter.la

#AsciiObsProvider_la_SOURCES = AsciiObsProvider.cpp AsciiObsProvider.h
#AsciiObsProvider_la_LIBADD   = $(top_srcdir)/src/opaq_core/libopaq.la

#ECMWFMeteoProvider_la_SOURCES = ECMWFMeteoProvider.cpp ECMWFMeteoProvider.h
#ECMWFMeteoProvider_la_LIBADD   = $(top_srcdir)/src/opaq_core/libopaq.la

IRCELMeteoProvider_la_SOURCES = IRCELMeteoProvider.cpp IRCELMeteoProvider.h
IRCELMeteoProvider_la_LIBADD   = $(top_srcdir)/src/opaq_core/libopaq.la

Hdf5Buffer_la_SOURCES = Hdf5Buffer.cpp Hdf5Buffer.h
Hdf5Buffer_la_LIBADD = $(top_srcdir)/src/opaq_core/libopaq.la

#Hdf5ForecastsProvider_la_SOURCES = Hdf5ForecastsProvider.cpp Hdf5ForecastsProvider.h
#Hdf5ForecastsProvider_la_LIBADD = $(top_srcdir)/src/opaq_core/libopaq.la

#Hdf5ForecastsStore_la_SOURCES = Hdf5ForecastsStore.cpp Hdf5ForecastsStore.h
#Hdf5ForecastsStore_la_LIBADD = $(top_srcdir)/src/opaq_core/libopaq.la

#Hdf5MappingsStore_la_SOURCES = Hdf5MappingsStore.cpp Hdf5MappingsStore.h
#Hdf5MappingsStore_la_LIBADD = $(top_srcdir)/src/opaq_core/libopaq.la

RioObsProvider_la_SOURCES = RioObsProvider.cpp RioObsProvider.h
RioObsProvider_la_LIBADD = $(top_srcdir)/src/opaq_core/libopaq.la

XMLAQNetProvider_la_SOURCES = XMLAQNetProvider.cpp XMLAQNetProvider.h
XMLAQNetProvider_la_LIBADD = $(top_srcdir)/src/opaq_core/libopaq.la

XmlGridProvider_la_SOURCES = XmlGridProvider.cpp XmlGridProvider.h
XmlGridProvider_la_LIBADD = $(top_srcdir)/src/opaq_core/libopaq.la

AsciiForecastWriter_la_SOURCES = AsciiForecastWriter.cpp AsciiForecastWriter.h
AsciiForecastWriter_la_LIBADD = $(top_srcdir)/src/opaq_core/libopaq.la
