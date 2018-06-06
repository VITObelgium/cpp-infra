find_package(unofficial-http-parser CONFIG REQUIRED)
find_package(restinio CONFIG REQUIRED)

add_executable(opaqpredictionservice main.cpp)
target_link_libraries(opaqpredictionservice restinio::restinio)

install(TARGETS opaqpredictionservice
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

