#!/usr/bin/env python3
import os
import sys
import sysconfig
import platform
import argparse

from deps.vcpkg.scripts.buildtools import vcpkg

if __name__ == "__main__":
    try:
        parser = argparse.ArgumentParser(
            description="Build infra.", parents=[vcpkg.build_argparser()]
        )
        parser.add_argument(
            "--ui", dest="ui_enabled", action="store_true", help="build the ui"
        )

        parser.add_argument(
            "--cpp20",
            dest="cpp20_enabled",
            action="store_true",
            help="build with C++20 support",
        )

        parser.add_argument(
            "--ci",
            dest="ci_build",
            action="store_true",
            help="continuous integration build",
        )

        parser.add_argument(
            "--build-config",
            dest="build_config",
            help="Build configuration (Debug or Release)",
            default="Release",
        )

        args = parser.parse_args()

        build_ui = "ON" if args.ui_enabled else "OFF"
        cxx_standard = "17" if (args.cpp20_enabled) else "17"
        sys_platform = sysconfig.get_platform()

        triplet = args.triplet
        if sys_platform == "win-amd64":
            triplet = "x64-windows-static-vs2022"
        elif sys_platform == "mingw":
            triplet = "x64-mingw"
        elif not triplet:
            triplet = vcpkg.prompt_for_triplet()

        cmake_args = [
            f"-DCMAKE_CXX_STANDARD={cxx_standard}",
            f"-DINFRA_UI_COMPONENTS={build_ui}",
            "-DINFRA_LOGGING=ON",
            "-DINFRA_GDAL=ON",
            "-DINFRA_XML=ON",
            "-DINFRA_TBB=ON",
            "-DINFRA_NUMERIC=OFF",
            "-DINFRA_CHARSET=ON",
            "-DINFRA_PROCESS=ON",
            "-DINFRA_DATABASE=ON",
            "-DINFRA_DATABASE_SQLITE=ON",
            "-DINFRA_DATABASE_POSTGRES=ON",
            "-DINFRA_DATABASE_HASHING=ON",
            "-DINFRA_COMPRESSION=ON",
            "-DINFRA_COMPRESSION_ZSTD=ON",
            "-DINFRA_ENABLE_TESTS=ON",
            "-DINFRA_CLI_PROGRESS=ON",
        ]

        if build_ui and triplet.startswith("x64-osx"):
            cmake_args.extend(
                [
                    "-DCMAKE_PREFIX_PATH=/usr/local/opt/qt6",
                    "-DVCPKG_ALLOW_SYSTEM_LIBS=ON",
                ]
            )

        targets = []
        build_dir = "infra"

        vcpkg.build_project(
            os.path.abspath(args.source_dir),
            triplet=triplet,
            cmake_args=cmake_args,
            targets=targets,
            build_name=build_dir,
            run_tests_after_build=args.run_tests,
            test_arguments=args.test_args,
            build_config=args.build_config,
        )
    except KeyboardInterrupt:
        print("\nInterrupted")
        sys.exit(-1)
    except RuntimeError as e:
        print(e)
        sys.exit(-1)
