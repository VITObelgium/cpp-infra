#!/usr/bin/env python3
import os
import sys
import sysconfig
import platform
import argparse

sys.path.insert(0, os.path.join(".", "deps", "vcpkg", "scripts"))
from buildtools import vcpkg

if __name__ == "__main__":
    try:
        parser = argparse.ArgumentParser(
            description="Build opaq.", parents=[vcpkg.build_argparser()]
        )
        parser.add_argument(
            "--ui", dest="ui_enabled", action="store_true", help="build the ui"
        )

        parser.add_argument(
            "--dist",
            dest="build_dist",
            action="store_true",
            help="build a release with proper git commit hash",
        )

        args = parser.parse_args()

        build_ui = "ON" if platform.system() == "Windows" or args.ui_enabled else "OFF"
        sys_platform = sysconfig.get_platform()

        triplet = None
        if sys_platform == "win-amd64":
            triplet = "x64-windows"
        elif sys_platform == "mingw":
            triplet = "x64-mingw"

        cmake_args = ["-DOPAQ_ENABLE_UI={}".format(build_ui), "-DSTATIC_QT=OFF", "-DCMAKE_BUILD_TYPE=Release"]

        if triplet == "x64-windows":
            cmake_args.extend(["-DCMAKE_PREFIX_PATH=C:/Qt/5.11.2/msvc2017_64"])
        elif triplet == "x64-mingw":
            cmake_args.extend(["-DSTATIC_QT=ON"])

        if args.build_dist:
            vcpkg.build_project_release(
                "opaq",
                os.path.abspath(args.source_dir),
                triplet=triplet,
                cmake_args=cmake_args,
            )
        else:
            vcpkg.build_project(
                "opaq",
                os.path.abspath(args.source_dir),
                triplet=triplet,
                cmake_args=cmake_args,
            )
    except KeyboardInterrupt:
        print("\nInterrupted")
        sys.exit(-1)
    except RuntimeError as e:
        print(e)
        sys.exit(-1)
