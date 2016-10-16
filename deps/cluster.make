SET(CMAKE_C_COMPILER x86_64-multilib-linux-musl-gcc)
SET(CMAKE_CXX_COMPILER x86_64-multilib-linux-musl-g++)

SET(HOST x86_64-multilib-linux-musl)

# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
