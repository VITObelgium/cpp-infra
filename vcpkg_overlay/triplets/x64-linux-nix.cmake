set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Linux)

set(LINKER_EXECUTABLE ld.gold)
set(VCPKG_LINKER_FLAGS "-static-libstdc++ -static-libgcc -fuse-ld=gold")
set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/toolchain-linux-nix.cmake")
