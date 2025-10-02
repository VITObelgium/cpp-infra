# detect the vcpkg triplet based on the system information
windows_triplet := "x64-windows-static"
vcpkg_version := "2025-09-03"
vcpkg_tool_url := if os_family() == "windows" {
    "https://github.com/microsoft/vcpkg-tool/releases/download/" + vcpkg_version + "/vcpkg.exe"
} else if os() == "macos" {
    "https://github.com/microsoft/vcpkg-tool/releases/download/" + vcpkg_version + "/vcpkg-macos"
} else {
    "https://github.com/microsoft/vcpkg-tool/releases/download/" + vcpkg_version + "/vcpkg-muslc"
}

VCPKG_DEFAULT_TRIPLET := if os_family() == "windows" {
    windows_triplet
} else if os() == "macos" {
    if arch() == "aarch64" {
        "arm64-osx-homebrew"
    } else { "x64-osx-homebrew" }
} else {
    "x64-linux"
}

cmake_preset := VCPKG_DEFAULT_TRIPLET

VCPKG_DEFAULT_HOST_TRIPLET := VCPKG_DEFAULT_TRIPLET
vcpkg_root := env('VCPKG_ROOT', join(justfile_directory(), "vcpkg"))
in_git_repo := path_exists(join(justfile_directory(), ".git"))

[windows]
download_vcpkg_bin:
    - rm -rf '{{join(justfile_directory(), "vcpkg")}}'
    mkdir '{{join(justfile_directory(), "vcpkg")}}'
    git clone --depth 1 https://github.com/microsoft/vcpkg.git '{{join(justfile_directory(), "vcpkg")}}'
    curl -L --fail --show-error '{{vcpkg_tool_url}}' -o '{{join(justfile_directory(), "vcpkg", "vcpkg.exe")}}'


[unix]
download_vcpkg_bin:
    - rm -rf '{{join(justfile_directory(), "vcpkg")}}'
    git clone --depth 1 https://github.com/microsoft/vcpkg.git '{{join(justfile_directory(), "vcpkg")}}'
    curl -L --fail --show-error '{{vcpkg_tool_url}}' -o '{{join(justfile_directory(), "vcpkg", "vcpkg")}}'
    chmod +x '{{join(justfile_directory(), "vcpkg", "vcpkg")}}'

git_status_clean:
    if {{in_git_repo}}; then git diff --quiet --exit-code; fi

[windows]
clear_cache_vs:
    rm '{{justfile_directory()}}/build/visualstudio/CMakeCache.txt'
    rm -rf '{{justfile_directory()}}/build/visualstudio/CMakeFiles'

clear_cache:
    rm '{{justfile_directory()}}/build/cmake/CMakeCache.txt'
    rm -rf '{{justfile_directory()}}/build/cmake/CMakeFiles'

# will invoke just build_dist but first activates the visual studio environment
[windows]
dist triplet=VCPKG_DEFAULT_TRIPLET git_hash=`git rev-parse HEAD`:
    cmd.exe /C '{{source_directory()}}/dist.bat' '{{justfile_directory()}}' '{{triplet}}' {{git_hash}}

[unix]
dist triplet=VCPKG_DEFAULT_TRIPLET git_hash=`git rev-parse HEAD`: (build_dist triplet)
