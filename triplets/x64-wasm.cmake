set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE static)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Emscripten)
if (APPLE)
    set(EMSCRIPTEN_ROOT /usr/local/opt/emscripten)
endif ()

if (EMSCRIPTEN_ROOT)
    if (EXISTS ${EMSCRIPTEN_ROOT}/libexec/cmake/Modules/Platform/Emscripten.cmake)
        include(${EMSCRIPTEN_ROOT}/libexec/cmake/Modules/Platform/Emscripten.cmake)
    else ()
        message(FATAL_ERROR "Could not find emscripten toolchain from the sdk: ${EMSCRIPTEN_ROOT}/libexec/cmake/Modules/Platform/Emscripten.cmake")
    endif ()
endif ()

set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)