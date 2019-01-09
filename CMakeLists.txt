include(CMakeDependentOption)
include(CMakePackageConfigHelpers)
include(CheckCXXSourceCompiles)
include(GNUInstallDirs)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set (INFRA_IS_TOPLEVEL ON)
endif ()

if (INFRA_IS_TOPLEVEL)
    cmake_minimum_required(VERSION 3.10)
    project(Infra
        VERSION 0.9.1
        LANGUAGES CXX
    )
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_EXTENSIONS OFF)

    # avoid windows specific warnings
    add_definitions(
        -D_CRT_SECURE_NO_WARNINGS
        -D_SCL_SECURE_NO_WARNINGS
        -D_SILENCE_CXX17_NEGATORS_DEPRECATION_WARNING
        -D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
        -D_SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING
    )
endif ()

option(INFRA_LOGGING "Enable logging functionality" ON)
option(INFRA_GDAL "Enable gdal wrapper for reading gis data" ON)
cmake_dependent_option(INFRA_EMBED_GDAL_DATA "Embed the required gdal data files in the binary" OFF "INFRA_GDAL" OFF)
option(INFRA_LICENSE_MANAGER "Enable the license management functionality" OFF)
option(INFRA_XML "Enable xml config reader" OFF)
option(INFRA_NUMERIC "Enable the numeric utility classes" OFF)
option(INFRA_ENABLE_TESTS "Build the unit tests" ON)
option(INFRA_ENABLE_BENCHMARKS "Build the micro benchmarks" OFF)
option(INFRA_ENABLE_DOCUMENTATION "Build the documentation" OFF)
option(INFRA_UI_COMPONENTS "Enable the generic qt ui components" OFF)
cmake_dependent_option(INFRA_UI_COMPONENTS_XLSX_EXPORT "Enable exporing qt item models to excel" OFF "INFRA_UI_COMPONENTS" OFF)
cmake_dependent_option(INFRA_UI_COMPONENTS_LOCATION "Location related qt ui components" ON "INFRA_UI_COMPONENTS;INFRA_GDAL" OFF)

check_cxx_source_compiles("
#include <filesystem>
namespace fs = std::filesystem;
int main() { return 0; }
" HAVE_FILESYSTEM_H)

check_cxx_source_compiles("
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
int main() { return 0; }
" HAVE_EXP_FILESYSTEM_H)

if (EMSCRIPTEN)
    # emscripten installs the headers but does not support the lib
    set (HAVE_EXP_FILESYSTEM_H OFF)
endif ()

if (NOT HAVE_FILESYSTEM_H AND NOT HAVE_EXP_FILESYSTEM_H)
    find_package(Boost COMPONENTS filesystem)
    if (TARGET Boost::filesystem)
        set (HAVE_BOOST_FILESYSTEM ON)
    endif ()
endif ()

if (HAVE_FILESYSTEM_H)
    message(STATUS "Found std::filesystem")
elseif (HAVE_EXP_FILESYSTEM_H)
    message(STATUS "Found std::experimental::filesystem")
elseif (HAVE_BOOST_FILESYSTEM)
    message(STATUS "Found boost::filesystem")
else ()
    message(WARNING "no filesystem implementation found")
endif ()

find_package(fmt CONFIG REQUIRED)
# make sure we use release versions of fmt lib when building relwithdebinfo
set_target_properties(fmt::fmt PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)

set(INFRA_PUBLIC_HEADERS
    include/infra/algo.h
    include/infra/cast.h
    include/infra/cell.h
    include/infra/enumutils.h
    include/infra/enumflags.h
    include/infra/generator.h
    include/infra/legend.h
    include/infra/legendscaletype.h
    include/infra/line.h
    include/infra/point.h
    include/infra/process.h
    include/infra/scopeguard.h
    include/infra/signal.h
    include/infra/string.h
    include/infra/exception.h
    include/infra/filesystem.h
    include/infra/color.h
    include/infra/colormap.h
    include/infra/rect.h
    include/infra/threadpool.h
    include/infra/typetraits.h
    include/infra/typetraits-private.h
    include/infra/inireader.h
)

add_library(infra
    legend.cpp
    legenddataanalyser.h legenddataanalyser.cpp
    process.cpp
    string.cpp
    exception.cpp
    filesystem.cpp
    color.cpp
    colormap.cpp
    threadpool.cpp
    inireader.cpp
)

add_library(Infra::Infra ALIAS infra)

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
elseif (HAVE_BOOST_FILESYSTEM)
    target_compile_definitions(infra PUBLIC HAVE_BOOST_FILESYSTEM)
endif ()

target_compile_definitions(infra PUBLIC FMT_STRING_ALIAS=1)

target_link_libraries(infra PUBLIC fmt::fmt)

if (HAVE_BOOST_FILESYSTEM)
    target_link_libraries(infra PUBLIC Boost::filesystem)
elseif (CMAKE_COMPILER_IS_GNUCXX)
    if (HAVE_FILESYSTEM_H OR HAVE_EXP_FILESYSTEM_H)
        target_link_libraries(infra PRIVATE stdc++fs)
    endif ()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if (HAVE_FILESYSTEM_H)
        find_library(CPP_FS_LIB c++fs)
        if (CPP_FS_LIB)
            target_link_libraries(infra PRIVATE ${CPP_FS_LIB})
        else ()
            message(FATAL_ERROR "c++fs library not found for clang")
        endif ()
    elseif (HAVE_EXP_FILESYSTEM_H)
        find_library(CPP_EXP_LIB c++experimental)
        if (CPP_EXP_LIB)
            target_link_libraries(infra PRIVATE ${CPP_EXP_LIB})
        else ()
            message(FATAL_ERROR "c++experimental library not found for clang")
        endif ()
    endif ()
endif ()

if (INFRA_LOGGING)
    find_package(spdlog CONFIG REQUIRED)
    list (APPEND INFRA_PUBLIC_HEADERS include/infra/log.h)
    target_sources(infra PRIVATE log.cpp)
    target_link_libraries(infra PUBLIC spdlog::spdlog)
    # make sure spdlog does not use its internal version of fmt
    target_compile_definitions(infra PUBLIC SPDLOG_FMT_EXTERNAL PRIVATE INFRA_LOG_ENABLED)
endif ()

if (INFRA_XML)
    find_package(pugixml CONFIG REQUIRED)
    list (APPEND INFRA_PUBLIC_HEADERS include/infra/xmldocument.h)
    target_sources(infra PRIVATE xmldocument.cpp)
    target_link_libraries(infra PRIVATE pugixml)
endif ()

if (INFRA_GDAL)
    find_package(Gdal REQUIRED)
    find_package(Gsl REQUIRED)
    find_package(date CONFIG REQUIRED)

    list (APPEND INFRA_PUBLIC_HEADERS
        include/infra/gdal.h
        include/infra/gdalalgo.h
        include/infra/gdalgeometry.h
        include/infra/geometadata.h
        include/infra/gdal-private.h
    )

    target_sources(infra PRIVATE
        gdal.cpp
        gdalalgo.cpp
        gdalgeometry.cpp
        geometadata.cpp
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

        target_compile_definitions(infra PRIVATE EMBED_GDAL_DATA)

        add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/gdal_data.h ${CMAKE_CURRENT_BINARY_DIR}/gdal_data.cpp
                           COMMAND ${CMAKE_COMMAND} ARGS -DGDAL_DATA_PATH=${Gdal_DATA_PATH} -DOUTPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/gdal_data -P ${CMAKE_CURRENT_SOURCE_DIR}/embedgdaldata.cmake
                           DEPENDS ${Gdal_DATA_PATH}/gcs.csv ${CMAKE_CURRENT_SOURCE_DIR}/embedgdaldata.cmake
                           COMMENT "Generate embedded gdal data"
        )
    endif ()

    if (INFRA_LOGGING)
        list (APPEND INFRA_PUBLIC_HEADERS include/infra/gdallog.h)
        target_sources(infra PRIVATE gdallog.cpp)
    endif ()
    target_link_libraries(infra PUBLIC Gdal::Gdal Gsl::Gsl date_interface)
endif ()

if (INFRA_NUMERIC)
    find_package(Eigen3 CONFIG REQUIRED)
    list (APPEND INFRA_PUBLIC_HEADERS include/infra/polyfit.h)
    target_link_libraries(infra PUBLIC Eigen3::Eigen)
endif ()

if (INFRA_LICENSE_MANAGER)
    find_package(Rlm CONFIG REQUIRED)
    list (APPEND INFRA_PUBLIC_HEADERS include/infra/licensemanager.h)
    target_sources(infra PRIVATE licensemanager.cpp)
    target_link_libraries(infra PRIVATE Rlm::Rlm)
endif ()

target_sources(infra PRIVATE ${INFRA_PUBLIC_HEADERS})
set_target_properties(infra PROPERTIES
    PUBLIC_HEADER "${INFRA_PUBLIC_HEADERS}"
)

set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/infra)

install(TARGETS infra EXPORT infra-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/infra
)

install(EXPORT infra-targets
    FILE InfraTargets.cmake
    NAMESPACE Infra::
    DESTINATION ${INSTALL_CONFIGDIR}
)

if (INFRA_IS_TOPLEVEL)
    write_basic_package_version_file("InfraConfigVersion.cmake"
        VERSION ${Infra_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/InfraConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/InfraConfig.cmake
        INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
    )

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/InfraConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/InfraConfigVersion.cmake
        DESTINATION ${INSTALL_CONFIGDIR}
    )
endif ()

if (INFRA_UI_COMPONENTS)
    find_package(Qt5Gui CONFIG REQUIRED)
    find_package(Qt5Widgets CONFIG REQUIRED)
    find_package(Qt5Core CONFIG REQUIRED)
    find_package(Qt5Location CONFIG REQUIRED)
    find_package(date CONFIG REQUIRED)

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
        include/uiinfra/clearableabstractitemmodel.h
        include/uiinfra/colorconversion.h
        include/uiinfra/coloredrectangledelegate.h ui/coloredrectangledelegate.cpp
        include/uiinfra/colorcombobox.h ui/colorcombobox.cpp
        include/uiinfra/colormapcombobox.h ui/colormapcombobox.cpp
        include/uiinfra/containerconversion.h
        include/uiinfra/cursorposprovider.h ui/cursorposprovider.cpp
        include/uiinfra/dateconversion.h
        include/uiinfra/multicombobox.h ui/multicombobox.cpp
        include/uiinfra/checkablecombobox.h ui/checkablecombobox.cpp
        include/uiinfra/checkableitemproxymodel.h ui/checkableitemproxymodel.cpp
        include/uiinfra/comboboxdelegate.h ui/comboboxdelegate.cpp
        include/uiinfra/delegatecombobox.h ui/delegatecombobox.cpp
        include/uiinfra/readonlyproxy.h
        include/uiinfra/mdisubwindow.h ui/mdisubwindow.cpp
        include/uiinfra/gradientdelegate.h ui/gradientdelegate.cpp
        include/uiinfra/scopedcursor.h
        include/uiinfra/toolboxview.h ui/toolboxview.cpp
        include/uiinfra/treemodel.h ui/treemodel.cpp
        include/uiinfra/userinteraction.h ui/userinteraction.cpp
        include/uiinfra/stringinputdialog.h ui/stringinputdialog.cpp
        include/uiinfra/stringconversion.h
        include/uiinfra/uniquevaluesproxymodel.h ui/uniquevaluesproxymodel.cpp
        include/uiinfra/fixeditemproxymodel.h ui/fixeditemproxymodel.cpp
        include/uiinfra/numericitemdelegates.h ui/numericitemdelegates.cpp
        include/uiinfra/sectionvisibilityselection.h ui/sectionvisibilityselection.cpp
        include/uiinfra/application.h ui/application.cpp
        include/uiinfra/qstringhash.h
        include/uiinfra/quickitemimagegrabber.h ui/quickitemimagegrabber.cpp
        include/uiinfra/tabletools.h ui/tabletools.cpp

        ui/toolboxitem.h ui/toolboxitem.cpp
        ui/qt5.natvis
        ${FORMS_HEADERS}
    )

    if (INFRA_UI_COMPONENTS_LOCATION)
         if (NOT INFRA_GDAL)
            message(FATAL_ERROR "INFRA_GDAL needs to be enabled when enabling INFRA_UI_COMPONENTS_LOCATION")
         endif ()

         find_package(Qt5QuickCompiler REQUIRED)
         qtquick_compiler_add_resources(INFRA_LOCATION_RESOURCES ui/qmlcontrols.qrc)

        target_sources(uiinfra PRIVATE
            ui/Legend.qml
            include/uiinfra/polygonmodel.h ui/polygonmodel.cpp
            include/uiinfra/polygonmodel2.h ui/polygonmodel2.cpp
            include/uiinfra/legendmodel.h ui/legendmodel.cpp
            include/uiinfra/maplegendmodel.h ui/maplegendmodel.cpp
            include/uiinfra/rastervalueprovider.h ui/rastervalueprovider.cpp
            ${INFRA_LOCATION_RESOURCES}
        )
    endif ()

    if (INFRA_UI_COMPONENTS_XLSX_EXPORT)
        find_package(XlsxWriter REQUIRED)
        target_sources(uiinfra PRIVATE
            include/uiinfra/modelexport.h ui/modelexport.cpp
        )
        target_link_libraries(uiinfra PRIVATE XslxWriter::XslxWriter)
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

        target_compile_definitions(infra PUBLIC HAVE_INFRA_LOG)
    endif ()

    set_target_properties(uiinfra PROPERTIES AUTOMOC ON)

    target_include_directories(uiinfra
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ui>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    )

    target_link_libraries(uiinfra PUBLIC Qt5::Gui Qt5::Widgets Qt5::Core Qt5::Location date_interface)

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

if (INFRA_ENABLE_DOCUMENTATION)
    add_subdirectory(docs)
endif ()
