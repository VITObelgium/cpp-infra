# Include boost
set (Boost_USE_STATIC_LIBS ON)
#set (Boost_USE_STATIC_RUNTIME ON)
add_definitions("-DBOOST_ALL_NO_LIB")
find_package(Boost REQUIRED COMPONENTS date_time system filesystem program_options)

find_package(Eigen3 REQUIRED)
find_package(TinyXml REQUIRED)
find_package(hdf5 CONFIG REQUIRED)
find_package(ZLIB REQUIRED)

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
    conf.cpp
    station.cpp
    network.cpp
    cell.cpp
    grid.cpp
    strfun.cpp
    obshandler.cpp
    dbqfile.cpp
    timeseries.cpp
    mapper.cpp
    idw_ipol.cpp
    krige_ipol.cpp
    rio_ipol.cpp
    correlationmodel.cpp
    exp_spcorr.cpp
    trendmodel.cpp
    polytrend.cpp
    xmltools.cpp
    parser.cpp
    output.cpp
    outputhandler.cpp
    asciiwriter.cpp
    ircelwriter.cpp
    apswriter.cpp 
    h5writer.cpp
    jsonwriter.cpp
)

if(MINGW)
    # mingw debug builds generate very big objects
    target_compile_options(rio PRIVATE "$<$<CONFIG:DEBUG>:-Wa,-mbig-obj>")
endif()

target_link_libraries(rio
    Eigen3::Eigen
    TinyXml::TinyXml
    Boost::boost
    Boost::date_time
    Boost::program_options
    Boost::system
    hdf5::hdf5_cpp-static
    ZLIB::ZLIB
)

add_executable(rioapp rio.cpp)
target_link_libraries(rioapp PRIVATE rio)
set_target_properties(rioapp PROPERTIES OUTPUT_NAME "rio")

install(TARGETS rioapp
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# add configure header to pass CMake settings
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/rioconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/rioconfig.h)
