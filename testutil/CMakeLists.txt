add_library(gdxtestutil
    include/gdx/test/comparisons.h
    include/gdx/test/platformsupport.h
    include/gdx/test/printsupport.h printsupport.cpp
    include/gdx/test/rasterasserts.h
    include/gdx/test/testbase.h
)

target_include_directories(gdxtestutil PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(gdxtestutil PUBLIC gdxcore)
