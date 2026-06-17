import 'vcpkg_overlay/vcpkg.just'

export VCPKG_OVERLAY_TRIPLETS := join(justfile_directory(), "vcpkg_overlay", "triplets")
export VCPKG_OVERLAY_PORTS := join(justfile_directory(), "vcpkg_overlay", "ports")

bootstrap:
    '{{ vcpkg_root }}/vcpkg' install --allow-unsupported \
            --triplet {{ VCPKG_DEFAULT_TRIPLET }} \
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
            --x-feature=testing \
            --x-install-root=build/vcpkgs

configure:
    cmake --preset {{ cmake_preset }}

build_debug: configure
    cmake --build ./build --config Debug

build_release: configure
    cmake --build ./build --config Release

build: build_release

build_dist triplet: build_release

test_debug: build
    ctest --test-dir ./build --output-on-failure -C Debug

test_release: build
    ctest --test-dir ./build --output-on-failure -C Release

test: test_release
