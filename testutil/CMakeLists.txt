find_package(doctest CONFIG REQUIRED)

set (INFRA_TESTUTIL_PUBLIC_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/infra/test/containerasserts.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/infra/test/printsupport.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/infra/test/reporter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/infra/test/tempdir.h
)

add_library(infratestutil
    ${INFRA_TESTUTIL_PUBLIC_HEADERS}
     printsupport.cpp
)

set_target_properties(infratestutil PROPERTIES
    PUBLIC_HEADER "${INFRA_TESTUTIL_PUBLIC_HEADERS}"
)

add_library(Infra::infratestutil ALIAS infratestutil)

target_include_directories(infratestutil PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(infratestutil PUBLIC infra doctest::doctest)

add_library(infratestutilmain testmain.cpp)
add_library(Infra::infratestutilmain ALIAS infratestutilmain)
target_compile_definitions(infratestutilmain PRIVATE 
    $<$<BOOL:${INFRA_TESTUTIL_MAIN_PROJDB_PATH}>:INFRA_TESTUTIL_MAIN_PROJDB_PATH="${INFRA_TESTUTIL_MAIN_PROJDB_PATH}">
)

target_link_libraries(infratestutilmain PUBLIC infratestutil)
