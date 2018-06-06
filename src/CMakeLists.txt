if (UNIX)
    set(hdf5_DIR ${CMAKE_PREFIX_PATH}/share/cmake)
endif ()

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Threads)
find_package(OpenSSL REQUIRED)

set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS system iostreams program_options)

if (MSVC)
    set(EXTRA_LIBS Psapi Ws2_32)
endif ()

list(APPEND EXTRA_LIBS ${CMAKE_THREAD_LIBS_INIT})

find_package(Eigen3 REQUIRED)

add_definitions(-D_XOPEN_SOURCE)

if (MSVC)
    # suppress decorated name length exceeded warning
    add_definitions(/wd4503)
endif ()

add_subdirectory(plugins)
add_subdirectory(opaq_core)

if (BUILD_UI)
    add_subdirectory(ui)
endif ()

add_executable(opaq opaq.cpp)
target_link_libraries(opaq opaqcore Eigen3::Eigen Boost::program_options opaqplugins)

install(TARGETS opaq
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

