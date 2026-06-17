import 'vcpkg_overlay/vcpkg.just'

export VCPKG_OVERLAY_TRIPLETS := join(justfile_directory(), "vcpkg_overlay", "triplets")
export VCPKG_OVERLAY_PORTS := join(justfile_directory(), "vcpkg_overlay", "ports")
export VCPKG_ROOT := vcpkg_root

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

[windows]
configure_vs:
    cmake --preset {{ cmake_preset }}-vs

build_debug: configure
    cmake --build --preset {{ cmake_preset }}-debug

build_release: configure
    cmake --build --preset {{ cmake_preset }}-release

[windows]
build_release_vs: configure_vs
    cmake --build --preset {{ cmake_preset }}-vs-release

build: build_release

[windows]
build_vs: build_release_vs

build_dist triplet: build_release

test_debug: build
    ctest --preset {{ cmake_preset }}-debug --output-on-failure -C Debug

test_release: build
    ctest --preset {{ cmake_preset }}-release --output-on-failure -C Release

test: test_release
