VCPKG_DEFAULT_TRIPLET := if os_family() == "windows" {
    "x64-windows-static"
} else if os() == "macos" {
    if arch() == "aarch64" {
        "arm64-osx"
    } else { "x64-osx" }
} else {
    "x64-linux"
}

cmake_preset := if os_family() == "windows" {
    "windows"
} else if os() == "macos" {
    if arch() == "aarch64" {
        "mac-arm"
    } else { "mac-intel" }
} else {
    "linux"
}

export VCPKG_ROOT := env('VCPKG_ROOT', "../vcpkg")

bootstrap:
    '{{VCPKG_ROOT}}/vcpkg' install --allow-unsupported --triplet {{VCPKG_DEFAULT_TRIPLET}} \
            --x-feature=cliprogress \
            --x-feature=process \
            --x-feature=hashing \
            --x-feature=xml \
            --x-feature=tbb \
            --x-feature=numeric \
            --x-feature=charset \
            --x-feature=compression \
            --x-feature=gdal \
            --x-feature=db \
            --x-feature=testing

configure: bootstrap
    cmake --preset {{cmake_preset}}

build_debug: configure
    cmake --build ./build --config Debug

build_release: configure
    cmake --build ./build --config Release

build: build_release

test_debug: build
    ctest --test-dir ./build --output-on-failure -C Debug

test_release: build
    ctest --test-dir ./build --output-on-failure -C Release

test: test_release