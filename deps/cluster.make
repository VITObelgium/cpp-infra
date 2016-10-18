set(CMAKE_COMMAND /tools/toolchains/x86_64-multilib-linux-musl/bin/cmake)
set(CMAKE_C_COMPILER /tools/toolchains/x86_64-multilib-linux-musl/bin/x86_64-multilib-linux-musl-gcc)
set(CMAKE_CXX_COMPILER /tools/toolchains/x86_64-multilib-linux-musl/bin/x86_64-multilib-linux-musl-g++)

set(HOST x86_64-multilib-linux-musl)

# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
