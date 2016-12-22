set(CMAKE_SYSTEM_NAME Windows)
set(HOST x86_64-w64-mingw32)

set(CROSS x86_64-w64-mingw32-)
set(CMAKE_C_COMPILER ${CROSS}gcc)
set(CMAKE_CXX_COMPILER ${CROSS}g++)
set(CMAKE_RC_COMPILER ${CROSS}windres)
set(CMAKE_Fortran_COMPILER ${CROSS}gfortran)

set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++ -B /tools/toolchains/x86_64-w64-mingw32/x86_64-w64-mingw32/sysroot/lib/" CACHE STRING "")
set(CMAKE_SHARED_LINKER_FLAGS "-static -static-libgcc -static-libstdc++" CACHE STRING "")
set(CMAKE_MODULE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++" CACHE STRING "")

# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
