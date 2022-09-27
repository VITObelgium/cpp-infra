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
        VERSION 0.11.2
        LANGUAGES CXX
    )
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_EXTENSIONS OFF)

    if (MSVC)
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
option(INFRA_PROCESS "Enable the process utility classes" OFF)
option(INFRA_DATABASE "Build the database utility classes" OFF)
option(INFRA_DATABASE_SQLITE "Build the database sqlite utility classes" OFF)
option(INFRA_DATABASE_POSTGRES "Build the database postgresql utility classes" OFF)
option(INFRA_HASHING "Build with hashing support" OFF)
option(INFRA_COMPRESSION "Build with compression support" OFF)
option(INFRA_COMPRESSION_ZSTD "Enable zstd compression support" OFF)
option(INFRA_ENABLE_TESTS "Build the unit tests" OFF)
option(INFRA_ENABLE_TEST_UTILS "Build the unit test utilities" ${INFRA_ENABLE_TESTS})
option(INFRA_ENABLE_BENCHMARKS "Build the micro benchmarks" OFF)
option(INFRA_ENABLE_DOCUMENTATION "Build the documentation" OFF)
option(INFRA_CLI_PROGRESS "Build the cli progress utility classes" OFF)
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
find_package(Gsl)
# make sure we use release versions of fmt lib when building relwithdebinfo
set_target_properties(fmt::fmt PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)
if (TARGET date::date-tz)
    set_target_properties(date::date-tz PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)
endif ()

set(INFRA_PUBLIC_HEADERS
    include/infra/algo.h
    include/infra/cast.h
    include/infra/cell.h
    include/infra/chrono.h
    include/infra/coordinate.h
    include/infra/cpuinfo.h
    include/infra/enumutils.h
    include/infra/enumflags.h
    include/infra/crs.h
    include/infra/csvwriter.h
    include/infra/demangle.h
    include/infra/generator.h
    include/infra/hash.h
    include/infra/interpolate.h
    include/infra/legend.h
    include/infra/legenddataanalyser.h 
    include/infra/legendscaletype.h
    include/infra/line.h
    include/infra/meteo.h
    include/infra/math.h
    include/infra/naturalbreaks.h
    include/infra/point.h
    include/infra/progressinfo.h
    include/infra/range.h
    include/infra/scopeguard.h
    include/infra/signal.h
    include/infra/string.h
    include/infra/exception.h
    include/infra/filesystem.h
    include/infra/color.h
    include/infra/colormap.h
    include/infra/conversion.h
    include/infra/environmentvariable.h
    include/infra/rect.h
    include/infra/size.h
    include/infra/span.h
    include/infra/threadpool.h
    include/infra/typeinfo.h
    include/infra/typetraits.h
    include/infra/typetraits-private.h
    include/infra/inireader.h
    include/infra/parallelstl.h
    include/infra/workerthread.h
)

add_library(infra
    chrono.cpp
    cpuinfo.cpp
    csvwriter.cpp
    demangle.cpp
    environmentvariable.cpp
    legend.cpp
    legenddataanalyser.cpp
    string.cpp
    exception.cpp
    filesystem.cpp
    color.cpp
    colormap.cpp
    threadpool.cpp
    inireader.cpp
    typeinfo.cpp
    workerthread.cpp
)

if (MSVC)
    set_source_files_properties(cliprogressbar.cpp PROPERTIES COMPILE_DEFINITIONS _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
endif ()

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

target_link_libraries(infra PUBLIC fmt::fmt date::date $<TARGET_NAME_IF_EXISTS:date::date-tz> $<TARGET_NAME_IF_EXISTS:Gsl::Gsl>)

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
    target_compile_definitions(infra PUBLIC INFRA_LOG_ENABLED)
    set_target_properties(spdlog::spdlog PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)
endif ()

if (INFRA_XML)
    find_package(pugixml CONFIG REQUIRED)
    set_target_properties(pugixml PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)
    list (APPEND INFRA_PUBLIC_HEADERS include/infra/xmldocument.h)
    target_sources(infra PRIVATE xmldocument.cpp)
    target_link_libraries(infra PRIVATE pugixml)
endif ()

if (INFRA_CLI_PROGRESS)
    find_package(indicators CONFIG REQUIRED)
    list (APPEND INFRA_PUBLIC_HEADERS include/infra/cliprogressbar.h)
    target_sources(infra PRIVATE cliprogressbar.cpp)
    target_link_libraries(infra PRIVATE indicators::indicators)
endif ()

if (INFRA_CHARSET)
    find_package(ICU COMPONENTS i18n data uc REQUIRED)
    list (APPEND INFRA_PUBLIC_HEADERS include/infra/charset.h)
    target_sources(infra PRIVATE charset.cpp)
    target_compile_definitions(infra PRIVATE U_USING_ICU_NAMESPACE U_STATIC_IMPLEMENTATION )
    target_link_libraries(infra PRIVATE ICU::i18n ICU::uc ICU::data)
endif ()

if (INFRA_PROCESS)
    find_package(reproc++ CONFIG REQUIRED)
    list (APPEND INFRA_PUBLIC_HEADERS include/infra/process.h)
    target_sources(infra PRIVATE process.cpp)
    target_link_libraries(infra PRIVATE reproc++)
    set_target_properties(reproc++ PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)
endif ()

if (INFRA_DATABASE)
    find_package(Sqlpp11 CONFIG REQUIRED)
    list (APPEND INFRA_PUBLIC_HEADERS include/infra/database.h)
    target_sources(infra PRIVATE database.cpp)
    target_link_libraries(infra PUBLIC sqlpp11::sqlpp11)
    if (INFRA_DATABASE_SQLITE)
        find_package(Sqlpp11 CONFIG REQUIRED COMPONENTS SQLite3)
        target_link_libraries(infra PUBLIC sqlpp11::sqlite3 )
        target_compile_definitions(infra PUBLIC INFRA_DB_SQLITE_SUPPORT)
    endif ()

    if (INFRA_DATABASE_POSTGRES)
        find_package(Sqlpp11 CONFIG REQUIRED COMPONENTS PostgreSQL)
        target_link_libraries(infra PUBLIC sqlpp11::postgresql)
        target_compile_definitions(infra PUBLIC INFRA_DB_POSTGRES_SUPPORT)
    endif ()
    
    if (INFRA_LOGGING)
        list (APPEND INFRA_PUBLIC_HEADERS include/infra/databaselog.h)
        target_sources(infra PRIVATE databaselog.cpp)
    endif ()
endif ()

if (INFRA_HASHING)
    find_package(cryptopp CONFIG REQUIRED)
    target_compile_definitions(infra PRIVATE INFRA_HASH_SUPPORT)
    target_link_libraries(infra PRIVATE cryptopp-static)
    list (APPEND INFRA_PUBLIC_HEADERS include/infra/crypto.h)
    target_sources(infra PRIVATE hash.cpp crypto.cpp)
endif ()

if (INFRA_COMPRESSION)
    list (APPEND INFRA_PUBLIC_HEADERS include/infra/compression.h)
    if (INFRA_COMPRESSION_ZSTD)
        find_package(zstd CONFIG REQUIRED)
        target_link_libraries(infra PUBLIC libzstd)
        target_compile_definitions(infra PUBLIC INFRA_COMPRESSION_ZSTD_SUPPORT ZSTD_STATIC_LINKING_ONLY)
    endif ()
endif ()

if (INFRA_GDAL)
    find_package(GDAL CONFIG)
    if (NOT TARGET GDAL::GDAL)
        find_package(GDAL REQUIRED)
        set(SYSTEM_GDAL ON)
        set_target_properties(GDAL::GDAL PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)
    endif ()

    if ("${GDAL_VERSION}" VERSION_GREATER_EQUAL 3 AND INFRA_EMBED_GDAL_DATA) 
        message(WARNING "Embedding gdal data is not supported for gdal versions >= 3")
    endif ()

    list (APPEND INFRA_PUBLIC_HEADERS
        include/infra/gdal.h
        include/infra/gdalalgo.h
        include/infra/gdalgeometry.h
        include/infra/gdalio.h
        include/infra/gdalcolortable.h
        include/infra/gdalspatialreference.h
        include/infra/geocoder.h
        include/infra/geometadata.h
        include/infra/gdal-private.h
    )

    target_sources(infra PRIVATE
        gdal.cpp
        gdalalgo.cpp
        gdalgeometry.cpp
        gdalio.cpp
        gdalspatialreference.cpp
        gdal-private.cpp
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
    elseif ("${GDAL_VERSION}" VERSION_GREATER_EQUAL 3 AND NOT SYSTEM_GDAL)
        add_custom_command(TARGET infra POST_BUILD
                           COMMENT "Copying proj db to the build directory"
                           COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:infra>/${INFRA_PROJDB_DIR}/"
                           COMMAND ${CMAKE_COMMAND} -E copy_if_different
                                $<$<CONFIG:Debug>:$<TARGET_FILE_DIR:PROJ::proj>/../../share/proj/proj.db>
                                $<$<NOT:$<CONFIG:Debug>>:$<TARGET_FILE_DIR:PROJ::proj>/../share/proj/proj.db>
                                "$<TARGET_FILE_DIR:infra>/${INFRA_PROJDB_DIR}/")
    endif ()
    

    if (INFRA_LOGGING)
        list (APPEND INFRA_PUBLIC_HEADERS include/infra/gdallog.h)
        target_sources(infra PRIVATE gdallog.cpp)
    endif ()

    if (INFRA_CHARSET)
        list (APPEND INFRA_PUBLIC_HEADERS include/infra/csvreader.h)
        target_sources(infra PRIVATE csvreader.cpp)
    endif ()

    target_link_libraries(infra PUBLIC GDAL::GDAL)
    if (TARGET GEOS::geos)
        set_target_properties(GEOS::geos PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)
    endif ()
    if (TARGET GEOS::geos_c)
        set_target_properties(GEOS::geos_c PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)
    endif ()
    set_target_properties(GDAL::GDAL PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)
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
    find_package(Qt6 COMPONENTS Core Gui Widgets Network OpenGL QUIET)
    if (NOT Qt6_FOUND)
        find_package(Qt5 5.15 COMPONENTS Core Gui Widgets Network OpenGL REQUIRED)
    endif ()

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
        include/uiinfra/noscrollspinbox.h
        include/uiinfra/checkablecombobox.h
        include/uiinfra/checkableitemproxymodel.h
        include/uiinfra/comboboxdelegate.h
        include/uiinfra/delegatecombobox.h
        include/uiinfra/readonlyproxy.h
        include/uiinfra/legendmodel.h
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
        ui/legendmodel.cpp
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
        ${INFRA_UI_FILES}
    )

    add_library(Infra::uiinfra ALIAS uiinfra)

    target_link_libraries(uiinfra PUBLIC
        Qt::Network
        Qt::Gui
        Qt::Widgets
        Qt::Core
        date::date
    )

    if (INFRA_UI_COMPONENTS_QML)
        if (Qt6_FOUND)
            find_package(Qt6 COMPONENTS Quick)
        else ()
            find_package(Qt5 5.15 COMPONENTS Quick REQUIRED)
        endif ()

        target_sources(uiinfra PRIVATE
            ui/pixmapimage.cpp
            ui/quickitemimagegrabber.cpp
        )

        list (APPEND UIINFRA_PUBLIC_HEADERS
            include/uiinfra/pixmapimage.h
            include/uiinfra/quickitemimagegrabber.h
        )

        target_link_libraries(uiinfra PUBLIC
            Qt::Quick
        )
    endif ()

    if (INFRA_UI_COMPONENTS_LOCATION)
        if (Qt6_FOUND)
            find_package(Qt6 COMPONENTS Location QuickCompiler)
        else ()
            find_package(Qt5 5.15 COMPONENTS Location QuickCompiler REQUIRED)
        endif ()
    
        if (NOT INFRA_GDAL)
            message(FATAL_ERROR "INFRA_GDAL needs to be enabled when enabling INFRA_UI_COMPONENTS_LOCATION")
        endif ()

        qtquick_compiler_add_resources(INFRA_LOCATION_RESOURCES ui/qmlcontrols.qrc)

        target_sources(uiinfra PRIVATE
            ui/Legend.qml
            ui/polygonmodel.cpp
            ui/maplegendmodel.cpp
            ui/rastervalueprovider.cpp
            ${INFRA_LOCATION_RESOURCES}
        )

        list (APPEND UIINFRA_PUBLIC_HEADERS
            include/uiinfra/polygonmodel.h
            include/uiinfra/maplegendmodel.h
            include/uiinfra/rastervalueprovider.h
        )

        target_link_libraries(uiinfra PUBLIC
            Qt::Location
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
    endif ()

    set_target_properties(uiinfra PROPERTIES AUTOMOC ON)
    set_target_properties(uiinfra PROPERTIES AUTORCC ON)
    set_target_properties(uiinfra PROPERTIES AUTOUIC ON)
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

    target_compile_definitions(uiinfra PUBLIC QT_NO_KEYWORDS)

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

if (INFRA_ENABLE_TEST_UTILS)
    add_subdirectory(testutil)
endif ()

if (INFRA_ENABLE_BENCHMARKS)
    add_subdirectory(benchmarks)
endif ()

if (INFRA_ENABLE_DOCUMENTATION)
    add_subdirectory(docs)
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

if (INFRA_INSTALL_DEVELOPMENT_FILES)
    install(TARGETS infra EXPORT InfraTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/infra
    )
    
    install(EXPORT InfraTargets
        NAMESPACE Infra::
        DESTINATION ${INSTALL_CONFIGDIR}
    )

    if (INFRA_ENABLE_TEST_UTILS)
        install(TARGETS infratestutil infratestutilmain EXPORT InfraTestTargets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/infra/test
        )
        
        install(EXPORT InfraTestTargets
            NAMESPACE Infra::
            DESTINATION ${INSTALL_CONFIGDIR}
        )
    endif ()

    if (INFRA_UI_COMPONENTS)
        install(TARGETS uiinfra EXPORT InfraUiTargets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/uiinfra
        )   

        install(EXPORT InfraUiTargets
            NAMESPACE Infra::
            DESTINATION ${INSTALL_CONFIGDIR}
        )
    endif ()
endif ()
