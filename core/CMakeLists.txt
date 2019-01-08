set(GDXCORE_PUBLIC_HEADERS
    include/gdx/eigenio.h
    include/gdx/line.h
    include/gdx/log.h
    include/gdx/point.h
    include/gdx/exception.h
    include/gdx/denseraster.h
    include/gdx/denserasterio.h
    include/gdx/maskedraster.h
    include/gdx/maskedrasterio.h
    include/gdx/maskedrasteriterator.h
    include/gdx/raster.h
    include/gdx/rasterarea.h
    include/gdx/rasterfwd.h
    include/gdx/rasterdiff.h
    include/gdx/rastercompare.h
    include/gdx/rasteriterator.h
    include/gdx/rastercelliterator.h
    include/gdx/rasterchecks.h
    include/gdx/sparseraster.h
    include/gdx/sparserasterio.h
    include/gdx/sparserasteriterator.h
    include/gdx/cpupredicates-private.h
    include/gdx/nodatapredicates-private.h
    include/gdx/eigeniterationsupport-private.h
    include/gdx/rasterio.h
    include/gdx/rasterspan.h
    include/gdx/rasterspanio.h
)

add_library(gdxcore
    ${GDXCORE_PUBLIC_HEADERS}
    rasterio.cpp
    raster.cpp
)

target_include_directories(gdxcore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

target_link_libraries(gdxcore PUBLIC infra gdxcommon gdxalgo Gsl::Gsl Eigen3::Eigen)
set_target_properties(gdxcore PROPERTIES PUBLIC_HEADER "${GDXCORE_PUBLIC_HEADERS}")

if (TARGET OpenMP::OpenMP_CXX)
    target_link_libraries(gdxcore PRIVATE OpenMP::OpenMP_CXX)
endif ()

if (GDX_INSTALL_DEVELOPMENT_FILES)
    install(TARGETS gdxcore EXPORT gdx-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gdx
    )
endif ()

if (MSVC)
    # add debugger visualisers for improved debugging info
    target_sources(gdxcore PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../GSL.natvis
        ${CMAKE_CURRENT_SOURCE_DIR}/../Eigen.natvis
    )
endif ()

if (GDX_ENABLE_TESTS)
    add_subdirectory(test)
endif ()
