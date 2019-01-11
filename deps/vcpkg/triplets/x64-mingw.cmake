set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE static)
set(VCPKG_LIBRARY_LINKAGE static)
set(CMAKE_SYSTEM_NAME Windows)
set(VCPKG_CMAKE_SYSTEM_NAME Windows)
set(MINGW ON CACHE BOOL "")
SET(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_EXE_LINKER_FLAGS "-static ${VCPKG_LINKER_FLAGS}" CACHE STRING "")
set(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} CACHE STRING "")
set(CMAKE_MODULE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} CACHE STRING "")

if (NOT CMAKE_HOST_WIN32)
set(HOST x86_64-w64-mingw32 CACHE STRING "")
set(CROSS ${HOST}-)
set(CMAKE_C_COMPILER ${CROSS}gcc CACHE STRING "")
set(CMAKE_CXX_COMPILER ${CROSS}g++ CACHE STRING "")
set(CMAKE_ASM_COMPILER ${CROSS}as CACHE STRING "")
set(CMAKE_RC_COMPILER ${CROSS}windres CACHE STRING "")
set(CMAKE_AR ${CROSS}ar CACHE STRING "")
set(CMAKE_RANLIB ${CROSS}ranlib CACHE STRING "")
set(CMAKE_Fortran_COMPILER ${CROSS}gfortran CACHE STRING "")
else ()
# for native mingw include .exe suffix (otherwise boost build fails)
set(CMAKE_C_COMPILER gcc.exe CACHE STRING "")
set(CMAKE_CXX_COMPILER g++.exe CACHE STRING "")
endif ()



if (APPLE)
set(CMAKE_SYSROOT /usr/local/opt/mingw-w64/toolchain-x86_64/x86_64-w64-mingw32)
elseif (UNIX)
set(CMAKE_SYSROOT /tools/toolchains/x86_64-w64-mingw32/x86_64-w64-mingw32/sysroot)
endif ()

set(CMAKE_FIND_ROOT_PATH ${CMAKE_PREFIX_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)
