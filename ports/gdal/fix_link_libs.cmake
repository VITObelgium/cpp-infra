message(STATUS "Patching GDAL for configured link lib: " ${GDAL_SRC_ROOT})

file(READ ${GDAL_SRC_ROOT}/makefile.vc conf_file)
# don't install the dll
string(REPLACE "install: $(GDAL_DLL) plugin_dir apps_dir " "install: plugin_dir apps_dir " conf_file "${conf_file}")
# copy static lib instead of import lib during install
string(REPLACE "copy gdal_i.lib $(LIBDIR)" "copy $(GDALLIB) $(LIBDIR)" conf_file "${conf_file}")
file(WRITE ${GDAL_SRC_ROOT}/makefile.vc "${conf_file}")

file(READ ${GDAL_SRC_ROOT}/apps/makefile.vc conf_file)
string(REPLACE "LIBS	=	$(GDALLIB)" "LIBS    =   $(GDALLIB) $(EXTERNAL_LIBS)" conf_file "${conf_file}")
file(WRITE ${GDAL_SRC_ROOT}/apps/makefile.vc "${conf_file}")

SET (MAKE_FILES
    apps/makefile.vc
    frmts/grib/makefile.vc
    frmts/mrsid/makefile.vc
    frmts/mrsid_lidar/makefile.vc
    frmts/sde/makefile.vc
    ogr/ogrsf_frmts/amigocloud/makefile.vc
    ogr/ogrsf_frmts/arcobjects/makefile.vc
    ogr/ogrsf_frmts/dwg/makefile.vc
    ogr/ogrsf_frmts/filegdb/makefile.vc
    ogr/ogrsf_frmts/libkml/makefile.vc
    ogr/ogrsf_frmts/mongodb/makefile.vc
    ogr/ogrsf_frmts/oci/makefile.vc
)

foreach(make_file IN LISTS MAKE_FILES)
    file(READ ${GDAL_SRC_ROOT}/${make_file} conf_file)
    # link applications against static lib and extenal libs instead of import lib
    string(REPLACE "$(GDAL_ROOT)\\gdal_i.lib" "$(GDALLIB) $(EXTERNAL_LIBS)" conf_file "${conf_file}")
    string(REPLACE "$(GDAL_ROOT)/gdal_i.lib" "$(GDALLIB) $(EXTERNAL_LIBS)" conf_file "${conf_file}")
    file(WRITE ${GDAL_SRC_ROOT}/${make_file} ${conf_file})
endforeach()

# fix detection of static geos
file(READ ${GDAL_SRC_ROOT}/configure conf_file)
string(REPLACE "-lgeos_c" "-lgeos_c -lgeos -lstdc++" conf_file "${conf_file}")
file(WRITE ${GDAL_SRC_ROOT}/configure "${conf_file}")