SET(CMAKE_SYSTEM_NAME Linux)
SET(HOST arm-linux-musl)

SET(CMAKE_C_COMPILER x86_64-linux-musl-gcc)
SET(CMAKE_CXX_COMPILER x86_64-linux-musl-g++)

# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
