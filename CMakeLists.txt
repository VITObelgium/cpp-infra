include(CMakeDependentOption)
include(CheckCXXSourceCompiles)

option(INFRA_GDAL "Enable gdal wrapper for reading gis data" ON)
option(INFRA_LOGGING "Enable logging functionality" ON)
option(INFRA_EMBED_GDAL_DATA "Embed the required gdal data files in the binary" ON)
option(INFRA_LICENSE_MANAGER "Enable the license management functionality" OFF)
option(INFRA_UI_COMPONENTS "Enable the generic qt ui components" OFF)
option(INFRA_XML "Enable xml config reader" OFF)
cmake_dependent_option(INFRA_UI_COMPONENTS_LOCATION "Location related qt ui components" ON "INFRA_UI_COMPONENTS" OFF)
option(INFRA_NUMERIC "Enable the numeric utility classes" OFF)
option(INFRA_ENABLE_TESTS "Build the unit tests" ON)
option(INFRA_ENABLE_BENCHMARKS "Build the micro benchmarks" OFF)

#check_cxx_source_compiles("
##include <filesystem>
#namespace fs = std::filesystem;
#int main() { return 0; }
#" HAVE_FILESYSTEM_H)

check_cxx_source_compiles("
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
int main() { return 0; }
" HAVE_EXP_FILESYSTEM_H)

if (HAVE_FILESYSTEM_H)
    message(STATUS "Found std::filesystem")
elseif (HAVE_EXP_FILESYSTEM_H)
    message(STATUS "Found std::experimental::filesystem")
else ()
    message(WARNING "no filesystem implementation found")
endif ()

find_package(fmt CONFIG REQUIRED)
# make sure we use release versions of fmt lib when building relwithdebinfo
set_target_properties(fmt::fmt PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)

add_library(infra
    include/infra/algo.h
    include/infra/cast.h
    include/infra/cell.h
    include/infra/enumutils.h
    include/infra/enumflags.h
    include/infra/generator.h
    include/infra/line.h
    include/infra/point.h
    include/infra/process.h process.cpp
    include/infra/signal.h
    include/infra/string.h string.cpp
    include/infra/exception.h exception.cpp
    include/infra/filesystem.h filesystem.cpp
    include/infra/color.h color.cpp
    include/infra/colormap.h colormap.cpp
    include/infra/threadpool.h threadpool.cpp
    include/infra/typetraits.h

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

if (HAVE_FILESYSTEM_H)
    target_compile_definitions(infra PUBLIC HAVE_FILESYSTEM_H)
elseif (HAVE_EXP_FILESYSTEM_H)
    target_compile_definitions(infra PUBLIC HAVE_EXP_FILESYSTEM_H)
endif ()

target_link_libraries(infra PRIVATE fmt::fmt)

if (CMAKE_COMPILER_IS_GNUCXX)
    if (HAVE_FILESYSTEM_H OR HAVE_EXP_FILESYSTEM_H)
        target_link_libraries(infra PRIVATE stdc++fs)
    endif ()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    find_library(CPP_EXP_LIB c++experimental)
    if (CPP_EXP_LIB)
        target_link_libraries(infra PRIVATE ${CPP_EXP_LIB})
    else ()
        message(FATAL_ERROR "c++experimental library not found for clang")
    endif ()
endif ()

if (INFRA_LOGGING)
    find_package(spdlog CONFIG REQUIRED)

    target_sources(infra PRIVATE include/infra/log.h log.cpp)
    target_link_libraries(infra PUBLIC spdlog::spdlog)
    # make sure spdlog does not use its internal version of fmt
    target_compile_definitions(infra PUBLIC SPDLOG_FMT_EXTERNAL)
endif ()

if (INFRA_XML)
    find_package(pugixml REQUIRED)
    target_sources(infra PRIVATE
        include/infra/configdocument.h configdocument.cpp
    )
    target_link_libraries(infra PRIVATE pugixml)
endif ()

if (INFRA_GDAL)
    find_package(Gdal REQUIRED)
    find_package(Gsl REQUIRED)

    target_sources(infra PRIVATE
        include/infra/gdal.h gdal.cpp
        include/infra/gdalalgo.h gdalalgo.cpp
        include/infra/gdalgeometry.h gdalgeometry.cpp
        include/infra/geometadata.h geometadata.cpp
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

    if (INFRA_LOGGING)
        target_sources(infra PRIVATE
            include/infra/gdallog.h gdallog.cpp
        )
    endif ()

    target_link_libraries(infra PUBLIC Gdal::Gdal Gsl::Gsl)
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
    target_link_libraries(infra PRIVATE Rlm::Rlm)
endif ()

if (INFRA_UI_COMPONENTS)
    find_package(Qt5Gui CONFIG REQUIRED)
    find_package(Qt5Widgets CONFIG REQUIRED)
    find_package(Qt5Core CONFIG REQUIRED)
    find_package(Qt5Location CONFIG REQUIRED)
    find_package(HinnantDate REQUIRED)

    set(AUTOGEN_SOURCE_GROUP moc)

    set(INFRA_UI_FILES
        ui/toolboxitem.ui
        ui/aboutdialog.ui
        ui/comboxtableview.ui
        ui/stringinputdialog.ui
    )
    if (INFRA_LOGGING)
        list(APPEND INFRA_UI_FILES ui/logview.ui)
    endif ()

    qt5_wrap_ui(FORMS_HEADERS ${INFRA_UI_FILES})

    add_library(uiinfra
        include/uiinfra/aboutdialog.h ui/aboutdialog.cpp
        include/uiinfra/dateconversion.h
        include/uiinfra/multicombobox.h ui/multicombobox.cpp
        include/uiinfra/checkablecombobox.h ui/checkablecombobox.cpp
        include/uiinfra/comboboxdelegate.h ui/comboboxdelegate.cpp
        include/uiinfra/readonlyproxy.h
        include/uiinfra/mdisubwindow.h ui/mdisubwindow.cpp
        include/uiinfra/scopedcursor.h
        include/uiinfra/toolboxview.h ui/toolboxview.cpp
        include/uiinfra/userinput.h ui/userinput.cpp
        include/uiinfra/stringinputdialog.h ui/stringinputdialog.cpp
        include/uiinfra/uniquevaluesproxymodel.h ui/uniquevaluesproxymodel.cpp
        include/uiinfra/fixeditemproxymodel.h ui/fixeditemproxymodel.cpp
        include/uiinfra/numericitemdelegates.h ui/numericitemdelegates.cpp
        include/uiinfra/sectionvisibilityselection.h ui/sectionvisibilityselection.cpp
        include/uiinfra/application.h ui/application.cpp
        ui/toolboxitem.h ui/toolboxitem.cpp
        ${FORMS_HEADERS}
    )

    if (INFRA_UI_COMPONENTS_LOCATION)
        target_sources(uiinfra PRIVATE
            include/uiinfra/polygonmodel.h ui/polygonmodel.cpp
        )
    endif ()

    if (INFRA_GDAL)
        target_sources(uiinfra PRIVATE
            include/uiinfra/polygonio.h ui/polygonio.cpp
        )
    endif ()

    if (INFRA_LOGGING)
        target_sources(uiinfra PRIVATE
            include/uiinfra/logview.h ui/logview.cpp
            include/uiinfra/logsinkmodel.h ui/logsinkmodel.cpp
        )

        target_compile_definitions(infratest PUBLIC HAVE_INFRA_LOG)
    endif ()

    set_target_properties(uiinfra PROPERTIES AUTOMOC ON)

    target_include_directories(uiinfra
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ui>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    )

    target_link_libraries(uiinfra PUBLIC Qt5::Gui Qt5::Widgets Qt5::Core Qt5::Location HinnantDate::Date)

    if (INFRA_LOGGING)
        target_link_libraries(uiinfra PUBLIC infra)
    endif ()
endif ()

if (INFRA_ENABLE_TESTS)
    add_subdirectory(test)
    if (INFRA_UI_COMPONENTS)
        add_subdirectory(ui/test)
    endif ()
endif ()

if (INFRA_ENABLE_BENCHMARKS)
    add_subdirectory(benchmarks)
endif ()
