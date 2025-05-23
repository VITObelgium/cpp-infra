if(NOT _VCPKG_LINUX_TOOLCHAIN)
set(_VCPKG_LINUX_TOOLCHAIN 1)
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_CROSSCOMPILING OFF CACHE BOOL "")
endif()
set(CMAKE_SYSTEM_NAME Linux CACHE STRING "")
if(VCPKG_TARGET_ARCHITECTURE STREQUAL "x64")
   set(CMAKE_SYSTEM_PROCESSOR x86_64 CACHE STRING "")
elseif(VCPKG_TARGET_ARCHITECTURE STREQUAL "x86")
   set(CMAKE_SYSTEM_PROCESSOR x86 CACHE STRING "")
   string(APPEND VCPKG_C_FLAGS " -m32")
   string(APPEND VCPKG_CXX_FLAGS " -m32")
elseif(VCPKG_TARGET_ARCHITECTURE STREQUAL "arm")
    set(CMAKE_SYSTEM_PROCESSOR armv7l CACHE STRING "")
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64")
        if(NOT DEFINED CMAKE_CXX_COMPILER)
            set(CMAKE_CXX_COMPILER "arm-linux-gnueabihf-g++")
        endif()
        if(NOT DEFINED CMAKE_C_COMPILER)
            set(CMAKE_C_COMPILER "arm-linux-gnueabihf-gcc")
        endif()
        message(STATUS "Cross compiling arm on host x86_64, use cross compiler: ${CMAKE_CXX_COMPILER}/${CMAKE_C_COMPILER}")
    endif()
elseif(VCPKG_TARGET_ARCHITECTURE STREQUAL "arm64")
    set(CMAKE_SYSTEM_PROCESSOR aarch64 CACHE STRING "")
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux"  AND CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64")
        if(NOT DEFINED CMAKE_CXX_COMPILER)
            set(CMAKE_CXX_COMPILER "aarch64-linux-gnu-g++")
        endif()
        if(NOT DEFINED CMAKE_C_COMPILER)
            set(CMAKE_C_COMPILER "aarch64-linux-gnu-gcc")
        endif()
        message(STATUS "Cross compiling arm64 on host x86_64, use cross compiler: ${CMAKE_CXX_COMPILER}/${CMAKE_C_COMPILER}")
    endif()
endif()

set(HOST x86_64-unknown-linux-gnu CACHE STRING "")
set(CROSS /tools/toolchains/${HOST}/bin/${HOST}-)

set(CMAKE_C_COMPILER ${CROSS}gcc CACHE FILEPATH "")
set(CMAKE_ASM_COMPILER ${CROSS}gcc CACHE FILEPATH "")
set(CMAKE_CXX_COMPILER ${CROSS}g++ CACHE FILEPATH "")
set(CMAKE_Fortran_COMPILER ${CROSS}gfortran CACHE FILEPATH "")
set(CMAKE_RANLIB ${CROSS}ranlib CACHE FILEPATH "")
set(CMAKE_STRIP ${CROSS}strip CACHE FILEPATH "")
set(CMAKE_NM ${CROSS}gcc-nm CACHE FILEPATH "")
set(CMAKE_AR ${CROSS}gcc-ar CACHE FILEPATH "")
set(CMAKE_RANLIB ${CROSS}gcc-ranlib CACHE FILEPATH "")
set(CMAKE_OBJDUMP ${CROSS}objdump CACHE FILEPATH "")
set(CMAKE_SYSROOT /tools/toolchains/${HOST}/${HOST}/sysroot CACHE PATH "")
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY CACHE STRING "")
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY CACHE STRING "")

get_property( _CMAKE_IN_TRY_COMPILE GLOBAL PROPERTY IN_TRY_COMPILE )
if(NOT _CMAKE_IN_TRY_COMPILE)
    string(APPEND CMAKE_C_FLAGS_INIT " -fPIC ${VCPKG_C_FLAGS} ")
    string(APPEND CMAKE_CXX_FLAGS_INIT " -fPIC ${VCPKG_CXX_FLAGS} ")
    string(APPEND CMAKE_C_FLAGS_DEBUG_INIT " ${VCPKG_C_FLAGS_DEBUG} ")
    string(APPEND CMAKE_CXX_FLAGS_DEBUG_INIT " ${VCPKG_CXX_FLAGS_DEBUG} ")
    string(APPEND CMAKE_C_FLAGS_RELEASE_INIT " ${VCPKG_C_FLAGS_RELEASE} ")
    string(APPEND CMAKE_CXX_FLAGS_RELEASE_INIT " ${VCPKG_CXX_FLAGS_RELEASE} ")

    string(APPEND CMAKE_SHARED_LINKER_FLAGS_INIT " ${VCPKG_LINKER_FLAGS} ")
    string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT " ${VCPKG_LINKER_FLAGS} ")
    if(VCPKG_CRT_LINKAGE STREQUAL "static")
        string(APPEND CMAKE_SHARED_LINKER_FLAGS_INIT "-static ")
        string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT "-static ")
    endif()
    string(APPEND CMAKE_SHARED_LINKER_FLAGS_DEBUG_INIT " ${VCPKG_LINKER_FLAGS_DEBUG} ")
    string(APPEND CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT " ${VCPKG_LINKER_FLAGS_DEBUG} ")
    string(APPEND CMAKE_SHARED_LINKER_FLAGS_RELEASE_INIT " ${VCPKG_LINKER_FLAGS_RELEASE} ")
    string(APPEND CMAKE_EXE_LINKER_FLAGS_RELEASE_INIT " ${VCPKG_LINKER_FLAGS_RELEASE} ")
endif()
endif()
