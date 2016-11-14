set(CMAKE_C_COMPILER x86_64-multilib-linux-musl-gcc)
set(CMAKE_CXX_COMPILER x86_64-multilib-linux-musl-g++)

set(HOST x86_64-multilib-linux-musl)

set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++" CACHE STRING "")
set(CMAKE_SHARED_LINKER_FLAGS "-static -static-libgcc -static-libstdc++" CACHE STRING "")
set(CMAKE_MODULE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++" CACHE STRING "")

# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
