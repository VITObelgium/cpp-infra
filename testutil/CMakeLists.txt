find_package(doctest CONFIG REQUIRED)

set (INFRA_TESTUTIL_PUBLIC_HEADERS
    include/infra/test/containerasserts.h
    include/infra/test/printsupport.h
    include/infra/test/reporter.h
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
target_compile_definitions(infratestutilmain PRIVATE $<$<BOOL:INFRA_GDAL>:HAVE_GDAL>)
target_link_libraries(infratestutilmain PUBLIC infratestutil)

if (INFRA_INSTALL_DEVELOPMENT_FILES)
    install(TARGETS infratestutil infratestutilmain EXPORT infra-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/infra/test
    )
endif ()
