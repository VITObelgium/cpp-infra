set (GDX_ALGO_TEST_SOURCES
    accufluxtest.cpp
    algorithmtest.cpp
    casttest.cpp
    choroplethtest.cpp
    filtertest.cpp
    logicaltest.cpp
    minimumtest.cpp
    nodatatest.cpp
    sumtest.cpp
    majoritytest.cpp
    maximumtest.cpp
    mathoperationstest.cpp
    normalisetest.cpp
    categoryiotest.cpp
    suminbuffertest.cpp
    sumwithintraveldistancetest.cpp
    reclasstest.cpp
    clusteridtest.cpp
    clustersizetest.cpp
    conditionalstest.cpp
    voronoitest.cpp
    main.cpp
)

add_executable(gdxalgotest
    ${GDX_ALGO_TEST_SOURCES}
)

target_compile_definitions(gdxalgotest
    PUBLIC $<$<CONFIG:Debug>:BOOST_COMPUTE_DEBUG_KERNEL_COMPILATION>
)

target_include_directories(gdxalgotest PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(gdxalgotest PRIVATE gdxtestutil gdxalgo gdxcore GTest::GTest GMock::GMock)
if (TARGET OpenMP::OpenMP_CXX)
    target_link_libraries(gdxalgotest PRIVATE OpenMP::OpenMP_CXX)
endif ()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/testconfig.h)

add_test(NAME gdxalgotest COMMAND gdxalgotest)
