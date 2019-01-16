# Include boost
set (Boost_USE_STATIC_LIBS ON)
#set (Boost_USE_STATIC_RUNTIME ON)
add_definitions("-DBOOST_ALL_NO_LIB")
find_package(Boost REQUIRED COMPONENTS date_time system filesystem program_options)

find_package(Eigen3 REQUIRED)
find_package(hdf5 CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(fmt CONFIG REQUIRED)

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
    message(STATUS "dir='${dir}'")
endforeach()


# select the -fsanitze=address om naar memory leaks te checken !
# see : https://www.phoronix.com/scan.php?page=news_item&px=MTIzOTU
# and : https://github.com/google/sanitizers
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-result -fsanitize=address -fno-omit-frame-pointer")

# add the executable
add_library(rio
    conf.hpp conf.cpp
    station.hpp station.cpp
    network.hpp network.cpp
    cell.hpp cell.cpp
    grid.hpp grid.cpp
    strfun.hpp strfun.cpp
    obshandler.hpp obshandler.cpp
    dbqfile.hpp dbqfile.cpp
    timeseries.hpp timeseries.cpp
    griddefinition.hpp
    gridmapper.hpp gridmapper.cpp
    mapper.hpp mapper.cpp
    idw_ipol.hpp idw_ipol.cpp
    krige_ipol.hpp krige_ipol.cpp
    rio_ipol.hpp rio_ipol.cpp
    correlationmodel.hpp correlationmodel.cpp
    exp_spcorr.hpp exp_spcorr.cpp
    trendmodel.hpp trendmodel.cpp
    polytrend.hpp polytrend.cpp
    mlrtrend.hpp mlrtrend.cpp
    modelrunner.hpp modelrunner.cpp
    xmltools.hpp xmltools.cpp
    parser.hpp parser.cpp
    output.hpp output.cpp
    outputhandler.hpp outputhandler.cpp
    asciiwriter.hpp asciiwriter.cpp
    ircelwriter.hpp ircelwriter.cpp
    memorywriter.hpp memorywriter.cpp
    apswriter.hpp apswriter.cpp
    h5writer.hpp h5writer.cpp
    jsonwriter.hpp jsonwriter.cpp
    ifdmwriter.hpp ifdmwriter.cpp
    rio.hpp
)

if(MINGW)
    # mingw debug builds generate very big objects
    target_compile_options(rio PRIVATE "$<$<CONFIG:DEBUG>:-Wa,-mbig-obj>")
endif()

target_include_directories(rio PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(rio
    infra
    Eigen3::Eigen
    Boost::boost
    Boost::date_time
    Boost::program_options
    Boost::iostreams
    Boost::system
    hdf5::hdf5_cpp-static
    ZLIB::ZLIB
    fmt::fmt
)

add_executable(rioapp rio.cpp)
target_link_libraries(rioapp PRIVATE rio)
if (MSVC)
    set_target_properties(rioapp PROPERTIES OUTPUT_NAME "riocli")
endif()

install(TARGETS rioapp
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# add configure header to pass CMake settings
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/rioconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/rioconfig.h)
