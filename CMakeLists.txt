include(CMakeDependentOption)
include(CMakePackageConfigHelpers)
include(CheckIncludeFileCXX)
include(CheckCXXSourceCompiles)
include(GNUInstallDirs)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set (INFRA_IS_TOPLEVEL ON)
endif ()

if (INFRA_IS_TOPLEVEL)
    cmake_minimum_required(VERSION 3.15)
    project(Infra
        VERSION 0.9.19
        LANGUAGES CXX
    )
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_EXTENSIONS OFF)

    if (MSVC)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        # avoid windows specific warnings
        add_definitions(
            -D_CRT_SECURE_NO_WARNINGS
            -D_SCL_SECURE_NO_WARNINGS
            -D_SILENCE_CXX17_NEGATORS_DEPRECATION_WARNING
            -D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
            -D_SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING
        )
    endif ()
endif ()

option(INFRA_LOGGING "Enable logging functionality" OFF)
option(INFRA_GDAL "Enable gdal wrapper for reading gis data" OFF)
cmake_dependent_option(INFRA_EMBED_GDAL_DATA "Embed the required gdal data files in the binary" OFF "INFRA_GDAL" OFF)
option(INFRA_LICENSE_MANAGER "Enable the license management functionality" OFF)
option(INFRA_XML "Enable xml config reader" OFF)
option(INFRA_NUMERIC "Enable the numeric utility classes" OFF)
option(INFRA_CHARSET "Enable the character set utility classes" OFF)
option(INFRA_DATABASE "Build the database utility classes" OFF)
option(INFRA_DATABASE_SQLITE "Build the database sqlite utility classes" OFF)
option(INFRA_DATABASE_POSTGRES "Build the database postgresql utility classes" OFF)
option(INFRA_ENABLE_TESTS "Build the unit tests" OFF)
option(INFRA_ENABLE_TEST_UTILS "Build the unit test utilities" ${INFRA_ENABLE_TESTS})
option(INFRA_ENABLE_BENCHMARKS "Build the micro benchmarks" OFF)
option(INFRA_ENABLE_DOCUMENTATION "Build the documentation" OFF)
option(INFRA_UI_COMPONENTS "Enable the generic qt ui components" OFF)
option(INFRA_INSTALL_DEVELOPMENT_FILES "Install the infra development files (headers/libs)" ${INFRA_IS_TOPLEVEL})
cmake_dependent_option(INFRA_UI_COMPONENTS_XLSX_EXPORT "Enable exporing qt item models to excel" OFF "INFRA_UI_COMPONENTS" OFF)
cmake_dependent_option(INFRA_UI_COMPONENTS_QML "Qml ui components" OFF "INFRA_UI_COMPONENTS;INFRA_GDAL" OFF)
cmake_dependent_option(INFRA_UI_COMPONENTS_LOCATION "Location related qt ui components" OFF "INFRA_UI_COMPONENTS;INFRA_GDAL;INFRA_UI_COMPONENTS_QML" OFF)

if (MSVC)
    set (CMAKE_REQUIRED_FLAGS /std:c++17)
else ()
    set (CMAKE_REQUIRED_FLAGS -std=c++17)
endif ()

check_include_file_cxx(charconv HAVE_CHARCONV_HEADER)

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

find_package(date CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
# make sure we use release versions of fmt lib when building relwithdebinfo
set_target_properties(fmt::fmt PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)
if (TARGET date::tz)
    set_target_properties(date::tz PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)
endif ()

set(INFRA_PUBLIC_HEADERS
    include/infra/algo.h
    include/infra/cast.h
    include/infra/cell.h
    include/infra/chrono.h
    include/infra/cliprogressbar.h
    include/infra/coordinate.h
    include/infra/cpuinfo.h
    include/infra/enumutils.h
    include/infra/enumflags.h
    include/infra/crs.h
    include/infra/csvwriter.h
    include/infra/generator.h
    include/infra/hash.h
    include/infra/interpolate.h
    include/infra/legend.h
    include/infra/legenddataanalyser.h 
    include/infra/legendscaletype.h
    include/infra/line.h
    include/infra/point.h
    include/infra/process.h
    include/infra/progressinfo.h
    include/infra/scopeguard.h
    include/infra/signal.h
    include/infra/string.h
    include/infra/exception.h
    include/infra/filesystem.h
    include/infra/color.h
    include/infra/colormap.h
    include/infra/conversion.h
    include/infra/rect.h
    include/infra/size.h
    include/infra/threadpool.h
    include/infra/typetraits.h
    include/infra/typetraits-private.h
    include/infra/inireader.h
    include/infra/parallelstl.h
)

add_library(infra
    chrono.cpp
    cpuinfo.cpp
    csvwriter.cpp
    legend.cpp
    legenddataanalyser.cpp
    process.cpp
    string.cpp
    exception.cpp
    filesystem.cpp
    color.cpp
    colormap.cpp
    threadpool.cpp
    inireader.cpp
)

add_library(Infra::infra ALIAS infra)

target_include_directories(infra
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
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

target_compile_definitions(infra
PUBLIC
    FMT_STRING_ALIAS=1
PRIVATE
    $<$<BOOL:${HAVE_CHARCONV_HEADER}>:HAVE_CHARCONV>
)

target_link_libraries(infra PUBLIC fmt::fmt date::date)

if (HAVE_BOOST_FILESYSTEM)
    target_link_libraries(infra PUBLIC Boost::filesystem)
elseif (CMAKE_COMPILER_IS_GNUCXX)
    if (HAVE_FILESYSTEM_H OR HAVE_EXP_FILESYSTEM_H)
        # in gcc >= 9.0 the filesystem library is inside libstdc++
        if ("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "9.0.0")
            target_link_libraries(infra PRIVATE stdc++fs)
        endif ()
    endif ()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND NOT CMAKE_HOST_WIN32)
    if (HAVE_FILESYSTEM_H)
        if (APPLE)
            find_library(CPP_FS_LIB c++fs)
            if (CPP_FS_LIB)
                target_link_libraries(infra PRIVATE ${CPP_FS_LIB})
            endif ()
        elseif (UNIX)
            # on linux libstdc++ is typically used
            target_link_libraries(infra PRIVATE stdc++fs)
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
    target_compile_definitions(infra PRIVATE INFRA_LOG_ENABLED)
    set_target_properties(spdlog::spdlog PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)
endif ()

if (INFRA_XML)
    find_package(pugixml CONFIG REQUIRED)
    set_target_properties(pugixml PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)
    list (APPEND INFRA_PUBLIC_HEADERS include/infra/xmldocument.h)
    target_sources(infra PRIVATE xmldocument.cpp)
    target_link_libraries(infra PRIVATE pugixml)
endif ()

if (INFRA_CHARSET)
    find_package(ICU COMPONENTS i18n data uc REQUIRED)
    list (APPEND INFRA_PUBLIC_HEADERS include/infra/charset.h)
    target_sources(infra PRIVATE charset.cpp)
    target_compile_definitions(infra PRIVATE U_USING_ICU_NAMESPACE U_STATIC_IMPLEMENTATION )
    target_link_libraries(infra PRIVATE ICU::i18n ICU::uc ICU::data)
endif ()

if (INFRA_DATABASE)
    find_package(Sqlpp11 REQUIRED)
    list (APPEND INFRA_PUBLIC_HEADERS include/infra/database.h)
    target_sources(infra PRIVATE database.cpp)
    target_link_libraries(infra PUBLIC Sqlpp11::Sqlpp11)
    if (INFRA_DATABASE_SQLITE)
        target_link_libraries(infra PUBLIC Sqlpp11::Sqlite3)
        target_compile_definitions(infra PUBLIC INFRA_DB_SQLITE_SUPPORT)
    endif ()

    if (INFRA_DATABASE_POSTGRES)
        target_link_libraries(infra PUBLIC Sqlpp11::Postgresql)
        target_compile_definitions(infra PUBLIC INFRA_DB_POSTGRES_SUPPORT)
    endif ()
    
    if (INFRA_LOGGING)
        list (APPEND INFRA_PUBLIC_HEADERS include/infra/databaselog.h)
        target_sources(infra PRIVATE databaselog.cpp)
    endif ()
endif ()

if (INFRA_GDAL)
    find_package(GDAL CONFIG)
    if (NOT TARGET GDAL::GDAL)
        find_package(Gdal REQUIRED)
    endif ()

    find_package(Gsl REQUIRED)

    list (APPEND INFRA_PUBLIC_HEADERS
        include/infra/gdal.h
        include/infra/gdalalgo.h
        include/infra/gdalgeometry.h
        include/infra/gdalio.h
        include/infra/gdalcolortable.h
        include/infra/geocoder.h
        include/infra/geometadata.h
        include/infra/gdal-private.h
    )

    target_sources(infra PRIVATE
        gdal.cpp
        gdalalgo.cpp
        gdalgeometry.cpp
        gdalio.cpp
        geocoder.cpp
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
    elseif (Gdal_ProjDB_DATA_PATH)
        add_custom_command(TARGET infra POST_BUILD
                           COMMENT "Copying proj4 db to the build directory"
                           COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:infra>/data
                           COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Gdal_ProjDB_DATA_PATH} $<TARGET_FILE_DIR:infra>/data
                           DEPENDS ${Gdal_ProjDB_DATA_PATH})
    endif ()
    

    if (INFRA_LOGGING)
        list (APPEND INFRA_PUBLIC_HEADERS include/infra/gdallog.h)
        target_sources(infra PRIVATE gdallog.cpp)
    endif ()

    if (INFRA_CHARSET)
        list (APPEND INFRA_PUBLIC_HEADERS include/infra/csvreader.h)
        target_sources(infra PRIVATE csvreader.cpp)
    endif ()

    target_link_libraries(infra PUBLIC $<TARGET_NAME_IF_EXISTS:GDAL::GDAL> $<TARGET_NAME_IF_EXISTS:Gdal::Gdal> Gsl::Gsl)
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

if (INFRA_UI_COMPONENTS)
    find_package(Qt5Gui REQUIRED)
    find_package(Qt5Widgets REQUIRED)
    find_package(Qt5Core REQUIRED)
    find_package(Qt5Network REQUIRED)
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

    set(UIINFRA_PUBLIC_HEADERS
        include/uiinfra/aboutdialog.h
        include/uiinfra/clearableabstractitemmodel.h
        include/uiinfra/colorconversion.h
        include/uiinfra/coloredrectangledelegate.h
        include/uiinfra/colorcombobox.h
        include/uiinfra/colormapcombobox.h
        include/uiinfra/containerconversion.h
        include/uiinfra/cursorposprovider.h
        include/uiinfra/dateconversion.h
        include/uiinfra/format.h
        include/uiinfra/multicombobox.h
        include/uiinfra/checkablecombobox.h
        include/uiinfra/checkableitemproxymodel.h
        include/uiinfra/comboboxdelegate.h
        include/uiinfra/delegatecombobox.h
        include/uiinfra/readonlyproxy.h
        include/uiinfra/mdisubwindow.h
        include/uiinfra/gradientdelegate.h
        include/uiinfra/scopedcursor.h
        include/uiinfra/toolboxview.h
        include/uiinfra/treemodel.h
        include/uiinfra/userinteraction.h
        include/uiinfra/stringinputdialog.h
        include/uiinfra/stringconversion.h
        include/uiinfra/uniquevaluesproxymodel.h
        include/uiinfra/fixeditemproxymodel.h
        include/uiinfra/numericitemdelegates.h
        include/uiinfra/sectionvisibilityselection.h
        include/uiinfra/application.h
        include/uiinfra/qstringhash.h
        include/uiinfra/tabletools.h
        include/uiinfra/doublespinboxwithinfinites.h
    )

    add_library(uiinfra
        ui/aboutdialog.cpp
        ui/coloredrectangledelegate.cpp
        ui/colorcombobox.cpp
        ui/colormapcombobox.cpp
        ui/cursorposprovider.cpp
        ui/multicombobox.cpp
        ui/checkablecombobox.cpp
        ui/checkableitemproxymodel.cpp
        ui/comboboxdelegate.cpp
        ui/delegatecombobox.cpp
        ui/mdisubwindow.cpp
        ui/gradientdelegate.cpp
        ui/toolboxview.cpp
        ui/treemodel.cpp
        ui/userinteraction.cpp
        ui/stringinputdialog.cpp
        ui/uniquevaluesproxymodel.cpp
        ui/fixeditemproxymodel.cpp
        ui/numericitemdelegates.cpp
        ui/sectionvisibilityselection.cpp
        ui/application.cpp
        ui/tabletools.cpp
        ui/doublespinboxwithinfinites.cpp
        ui/toolboxitem.cpp
        ui/qt5.natvis
        ${FORMS_HEADERS}
    )

    add_library(Infra::uiinfra ALIAS uiinfra)

    target_link_libraries(uiinfra PUBLIC
        Qt5::Network
        Qt5::Gui
        Qt5::Widgets
        Qt5::Core
        date::date
    )

    if (STATIC_QT AND WIN32)
        get_target_property(QT_NETWORK_FEATURES Qt5::Network INTERFACE_QT_ENABLED_FEATURES)
        if ("ssl" IN_LIST QT_NETWORK_FEATURES)
            message(STATUS "Qt network SSL enabled")
            target_link_libraries(uiinfra PRIVATE Crypt32.lib)
            if (Qt5Network_VERSION VERSION_GREATER_EQUAL 5.13.0)
                target_link_libraries(uiinfra PRIVATE Bcrypt.lib Secur32.lib Iphlpapi.lib)
            endif ()
        endif ()
    endif ()

    if (INFRA_UI_COMPONENTS_QML)
        find_package(Qt5Quick CONFIG REQUIRED)

        target_sources(uiinfra PRIVATE
            include/uiinfra/pixmapimage.h ui/pixmapimage.cpp
            include/uiinfra/quickitemimagegrabber.h ui/quickitemimagegrabber.cpp
        )

        target_link_libraries(uiinfra PUBLIC
            Qt5::Quick
        )
    endif ()

    if (INFRA_UI_COMPONENTS_LOCATION)
        find_package(Qt5Location CONFIG REQUIRED)
    
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

        target_link_libraries(uiinfra PUBLIC
            Qt5::Location
        )

        if (INFRA_GDAL)
            list (APPEND UIINFRA_PUBLIC_HEADERS include/uiinfra/polygonio.h)
            target_sources(uiinfra PRIVATE ui/polygonio.cpp)
        endif ()
    endif ()

    if (INFRA_UI_COMPONENTS_XLSX_EXPORT)
        find_package(XlsxWriter REQUIRED)
        list (APPEND UIINFRA_PUBLIC_HEADERS include/uiinfra/modelexport.h)
        target_sources(uiinfra PRIVATE ui/modelexport.cpp)
        target_link_libraries(uiinfra PRIVATE XlsxWriter::XlsxWriter)
    endif ()

    if (INFRA_LOGGING)
        list (APPEND UIINFRA_PUBLIC_HEADERS
            include/uiinfra/logview.h
            include/uiinfra/logsinkmodel.h 
        )

        target_sources(uiinfra PRIVATE
            ui/logview.cpp
            ui/logsinkmodel.cpp
        )

        target_compile_definitions(infra PUBLIC HAVE_INFRA_LOG)
    endif ()

    set_target_properties(uiinfra PROPERTIES AUTOMOC ON)
    target_sources(uiinfra PRIVATE ${UIINFRA_PUBLIC_HEADERS})
    set_target_properties(uiinfra PROPERTIES PUBLIC_HEADER "${UIINFRA_PUBLIC_HEADERS}")

    target_include_directories(uiinfra
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ui>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    )

    if (INFRA_LOGGING)
        target_link_libraries(uiinfra PUBLIC infra)
    endif ()
endif ()

if (INFRA_IS_TOPLEVEL)
    write_basic_package_version_file("InfraConfigVersion.cmake"
        VERSION ${Infra_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/InfraConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/InfraConfig.cmake
        INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
    )

    if (INFRA_INSTALL_DEVELOPMENT_FILES)
        install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/InfraConfig.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/InfraConfigVersion.cmake
            DESTINATION ${INSTALL_CONFIGDIR}
        )
    endif ()
endif ()

if (INFRA_ENABLE_TESTS)
    add_subdirectory(test)
    if (INFRA_UI_COMPONENTS)
        add_subdirectory(ui/test)
    endif ()
endif ()

if (INFRA_ENABLE_TEST_UTILS)
    add_subdirectory(testutil)
endif ()

if (INFRA_ENABLE_BENCHMARKS)
    add_subdirectory(benchmarks)
endif ()

if (INFRA_ENABLE_DOCUMENTATION)
    add_subdirectory(docs)
endif ()

if (INFRA_INSTALL_DEVELOPMENT_FILES)
    install(TARGETS infra EXPORT infra-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/infra
    )

    if (INFRA_UI_COMPONENTS)
        install(TARGETS uiinfra EXPORT infra-targets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/uiinfra
        )   
    endif ()

    install(EXPORT infra-targets
        FILE InfraTargets.cmake
        NAMESPACE Infra::
        DESTINATION ${INSTALL_CONFIGDIR}
    )
endif ()
