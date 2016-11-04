add_library(idwmodel STATIC
    InverseDistanceWeighting.h
    InverseDistanceWeighting.cpp
)

target_link_libraries(idwmodel opaqcore)

if (NOT STATIC_PLUGINS)
    set_target_properties(idwmodel PROPERTIES POSITION_INDEPENDENT_CODE ON)

    add_library(idwmodel MODULE InverseDistanceWeightingModelPlugin.cpp)

    target_link_libraries(idwmodelplugin idwmodel)
endif ()

set (PLUGINS ${PLUGINS} idwmodel PARENT_SCOPE)
