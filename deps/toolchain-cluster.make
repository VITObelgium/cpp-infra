set(CROSS x86_64-unknown-linux-gnu-)

set(CMAKE_C_COMPILER ${CROSS}gcc)
set(CMAKE_CXX_COMPILER ${CROSS}g++)
set(CMAKE_LINKER ${CROSS}ld CACHE FILEPATH "")
set(CMAKE_RANLIB ${CROSS}ranlib CACHE FILEPATH "")
set(CMAKE_STRIP ${CROSS}strip CACHE FILEPATH "")
set(CMAKE_NM ${CROSS}nm CACHE FILEPATH "")
set(CMAKE_AR ${CROSS}ar CACHE FILEPATH "")
set(CMAKE_RANLIB ${CROSS}ranlib CACHE FILEPATH "")
set(CMAKE_OBJDUMP ${CROSS}objdump CACHE FILEPATH "")

set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc" CACHE STRING "")
set(CMAKE_SHARED_LINKER_FLAGS "-static-libstdc++ -static-libgcc" CACHE STRING "")
set(CMAKE_MODULE_LINKER_FLAGS "-static-libstdc++ -static-libgcc" CACHE STRING "")

# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
