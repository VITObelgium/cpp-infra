add_executable(gdxinfo EXCLUDE_FROM_ALL gdxinfo.cpp)
add_executable(gdxcmp EXCLUDE_FROM_ALL gdxcmp.cpp)
add_executable(gdxconv EXCLUDE_FROM_ALL gdxconv.cpp)

if (WIN32)
    target_sources(gdxinfo PRIVATE ${CMAKE_BINARY_DIR}/version.rc)
    target_sources(gdxcmp PRIVATE ${CMAKE_BINARY_DIR}/version.rc)
    target_sources(gdxconv PRIVATE ${CMAKE_BINARY_DIR}/version.rc)
endif ()

target_link_libraries(gdxinfo gdxcore gdxalgo)
target_link_libraries(gdxcmp gdxcore gdxalgo)
target_link_libraries(gdxconv gdxcore gdxalgo)

add_custom_target(tools DEPENDS gdxinfo gdxcmp gdxconv)
