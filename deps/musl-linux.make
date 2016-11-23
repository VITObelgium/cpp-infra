set(CROSS_COMPILE x86_64-multilib-linux-musl-)
set(CMAKE_C_COMPILER ${CROSS_COMPILE}gcc)
set(CMAKE_CXX_COMPILER ${CROSS_COMPILE}g++)

set(HOST x86_64-multilib-linux-musl)

set(CMAKE_CXX_FLAGS "-flto" CACHE STRING "")
set(CMAKE_EXE_LINKER_FLAGS "-flto -static" CACHE STRING "")
set(CMAKE_SHARED_LINKER_FLAGS "-flto -static" CACHE STRING "")
set(CMAKE_MODULE_LINKER_FLAGS "-flto -static" CACHE STRING "")

# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
