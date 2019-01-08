add_library(gdxcommon INTERFACE)

if(MSVC)
    # make sure visual studio creates a project with the sources
    add_custom_target(gdxcommon.headeronly SOURCES
        include/gdx/cell.h
        include/gdx/line.h
        include/gdx/log.h
        include/gdx/rastermetadata.h
        include/gdx/point.h
    )
endif()

target_include_directories(gdxcommon
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include
)

if (GDX_INSTALL_DEVELOPMENT_FILES)
    install(TARGETS gdxcommon EXPORT gdx-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gdx
    )
endif ()

target_link_libraries(gdxcommon INTERFACE infra)
