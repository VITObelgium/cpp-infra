option(INFRA_GDAL "Enable gdal wrapper for reading gis data" ON)
option(INFRA_LOGGING "Enable logging functionality" ON)
option(INFRA_EMBED_GDAL_DATA "Embed the required gdal data files in the binary" ON)
option(INFRA_LICENSE_MANAGER "Enable the license management functionality" OFF)
option(INFRA_UI_COMPONENTS "Enable the generic qt ui components" OFF)
option(INFRA_NUMERIC "Enable the numeric utility classes" OFF)
option(INFRA_ENABLE_TESTS "Build the unit tests" ON)

find_package(fmt CONFIG REQUIRED)

add_library(infra
    include/infra/algo.h
    include/infra/cast.h
    include/infra/cell.h
    include/infra/enumutils.h
    include/infra/enumflags.h
    include/infra/generator.h
    include/infra/line.h
    include/infra/point.h
    include/infra/signal.h
    include/infra/string.h string.cpp
    include/infra/exception.h
    include/infra/filesystem.h filesystem.cpp
    include/infra/color.h color.cpp
    include/infra/colormap.h colormap.cpp
    include/infra/geometadata.h geometadata.cpp

    include/infra/internal/traits.h
    include/infra/inireader.h inireader.cpp
)

target_include_directories(infra
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

target_link_libraries(infra PRIVATE fmt::fmt)

if (CMAKE_COMPILER_IS_GNUCXX)
    target_link_libraries(infra PRIVATE stdc++fs)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    find_library(CPP_EXP_LIB c++experimental HINTS /usr/local/opt/llvm/lib)
    target_link_libraries(infra PRIVATE ${CPP_EXP_LIB})
endif ()

if (INFRA_GDAL)
    find_package(Gdal REQUIRED)
    find_package(Gsl REQUIRED)

    target_sources(infra PRIVATE
        include/infra/gdal.h gdal.cpp
        include/infra/internal/gdalinternal.h
    )

    if (INFRA_EMBED_GDAL_DATA)
        if (NOT Gdal_DATA_PATH)
            message(FATAL_ERROR "Gdal data path location not set for embedding data")
        endif ()

        target_sources(infra PRIVATE
            embedgdaldata.h
            embedgdaldata.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/gdal_data.h
            ${CMAKE_CURRENT_BINARY_DIR}/gdal_data.cpp
            embedgdaldata.cmake
        )

        target_compile_definitions(infra PRIVATE -DEMBED_GDAL_DATA)

        add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/gdal_data.h ${CMAKE_CURRENT_BINARY_DIR}/gdal_data.cpp
                           COMMAND ${CMAKE_COMMAND} ARGS -DGDAL_DATA_PATH=${Gdal_DATA_PATH} -DOUTPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/gdal_data -P ${CMAKE_CURRENT_SOURCE_DIR}/embedgdaldata.cmake
                           DEPENDS ${Gdal_DATA_PATH}/gcs.csv ${CMAKE_CURRENT_SOURCE_DIR}/embedgdaldata.cmake
                           COMMENT "Generate embedded gdal data"
        )
    endif ()

    target_link_libraries(infra PUBLIC Gdal::Gdal Gsl::Gsl)
endif ()

if (INFRA_LOGGING)
    find_package(spdlog CONFIG REQUIRED)

    target_sources(infra PRIVATE include/infra/log.h log.cpp)
    target_link_libraries(infra PUBLIC spdlog::spdlog)
endif ()

if (INFRA_NUMERIC)
    find_package(Eigen3 CONFIG REQUIRED)

    target_sources(infra PRIVATE include/infra/polyfit.h)
    target_link_libraries(infra PUBLIC Eigen3::Eigen)
endif ()

if (INFRA_LICENSE_MANAGER)
    find_package(Rlm CONFIG REQUIRED)

    target_sources(infra PRIVATE
        include/infra/licensemanager.h licensemanager.cpp
    )
endif ()

if (INFRA_UI_COMPONENTS)
    find_package(Qt5Gui CONFIG REQUIRED)
    find_package(Qt5Widgets CONFIG REQUIRED)

    set(AUTOGEN_SOURCE_GROUP moc)

    qt5_wrap_ui(FORMS_HEADERS ui/toolboxitem.ui ui/aboutdialog.ui)

    add_library(uiinfra
        include/uiinfra/aboutdialog.h ui/aboutdialog.cpp
        include/uiinfra/readonlyproxy.h
        include/uiinfra/mdisubwindow.h ui/mdisubwindow.cpp
        include/uiinfra/scopedcursor.h
        include/uiinfra/toolboxview.h ui/toolboxview.cpp
        include/uiinfra/userinput.h
        include/uiinfra/uniquevaluesproxymodel.h ui/uniquevaluesproxymodel.cpp
        ui/toolboxitem.h ui/toolboxitem.cpp
        ${FORMS_HEADERS}
    )

    set_target_properties(uiinfra PROPERTIES AUTOMOC ON)

    target_include_directories(uiinfra
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ui>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    )

    target_link_libraries(uiinfra PRIVATE Qt5::Gui Qt5::Widgets)
endif ()

if (INFRA_ENABLE_TESTS)
    add_subdirectory(test)
endif ()
