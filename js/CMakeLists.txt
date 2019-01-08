find_program (EMCC emcc)
if (NOT EMCC)
    message(FATAL_ERROR "Could not find emcc executable")
endif ()

add_library(gdxjs SHARED
    gdxjs.cpp
    jsadapters.h
    jsadapters.cpp
)

target_link_libraries(gdxjs PRIVATE
    gdxutil
    gdxalgo
)

set_target_properties(gdxjs PROPERTIES SUFFIX .bc)

get_target_property(WASM_DIR gdxjs ARCHIVE_OUTPUT_DIRECTORY)
get_target_property(GDX_WASM_MODULE gdxjs OUTPUT_NAME)

add_custom_command(TARGET gdxjs
                   POST_BUILD
                   COMMAND ${EMCC}
                   ARGS
                       -O3
                       -s WASM=1
                       -s TOTAL_MEMORY=96468992
                       -s DISABLE_EXCEPTION_CATCHING=0
                       --llvm-lto 1
                       --bind
                       $<TARGET_FILE:gdxjs>
                       -o $<TARGET_FILE_DIR:gdxjs>/gdx.js
                       # --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/assets@/assets
                   BYPRODUCTS
                       ${CMAKE_BINARY_DIR}/bin/gdx.js
                       ${CMAKE_BINARY_DIR}/bin/gdx.wasm
                       ${CMAKE_BINARY_DIR}/bin/gdx.data
                   COMMENT "Generating webassembly"
)