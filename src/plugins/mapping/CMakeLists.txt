add_library(idwmodel STATIC
    InverseDistanceWeighting.h
    InverseDistanceWeighting.cpp
)

target_link_libraries(idwmodel opaqcore)

set (PLUGINS ${PLUGINS} idwmodel PARENT_SCOPE)
