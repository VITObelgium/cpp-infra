find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Threads)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system iostreams program_options)

if (MSVC)
    set(EXTRA_LIBS Psapi Ws2_32)
endif ()

list(APPEND EXTRA_LIBS ${CMAKE_THREAD_LIBS_INIT})

find_package(Eigen3 REQUIRED)

if (MSVC)
    # suppress decorated name length exceeded warning
    add_definitions(/wd4503)
endif ()

add_subdirectory(rio)
add_subdirectory(plugins)
add_subdirectory(opaq_core)

if (OPAQ_ENABLE_UI)
    add_subdirectory(ui)
endif ()

if (OPAQ_ENABLE_PREDICTION_SERVICE)
    add_subdirectory(predictionservice)
endif ()

add_executable(opaq opaq.cpp)
target_link_libraries(opaq opaqcore Eigen3::Eigen Boost::program_options opaqplugins)

install(TARGETS opaq
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

