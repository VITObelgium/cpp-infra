# restinio depends on the unofficial-http-parser target to be present
find_package(unofficial-http-parser CONFIG REQUIRED)
find_package(restinio CONFIG REQUIRED)

add_executable(opaqpredictionservice main.cpp)
target_link_libraries(opaqpredictionservice PRIVATE
    restinio::restinio
    Boost::disable_autolinking
)

install(TARGETS opaqpredictionservice
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

