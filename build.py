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
        parser = argparse.ArgumentParser(description="Build opaq.", parents=[vcpkg.build_argparser()])
        parser.add_argument("--ui", dest="ui_enabled", action="store_true", help="build the ui")

        parser.add_argument(
            "--dist", dest="build_dist", action="store_true", help="build a release with proper git commit hash"
        )

        parser.add_argument(
            "--static", dest="static_runtime", default=False, action="store_true", help="Use the static runtime (windows only)"
        )

        args = parser.parse_args()

        build_ui = "ON" if platform.system() == "Windows" or args.ui_enabled else "OFF"
        sys_platform = sysconfig.get_platform()

        triplet = None
        if sys_platform == "win-amd64":
            triplet = "x64-windows-static" if args.static_runtime else "x64-windows"
        elif sys_platform == "mingw":
            triplet = "x64-mingw"
        else:
            triplet = vcpkg.prompt_for_triplet()

        cmake_args = ["-DOPAQ_ENABLE_UI={}".format(build_ui)]

        if build_ui:
            if triplet == "x64-windows":
                cmake_args.append("-DCMAKE_PREFIX_PATH=C:/Qt/5.12.0/msvc2017_64")
            elif triplet == "x64-mingw" or triplet == "x64-windows-static" or triplet.startswith("x64-osx"):
                cmake_args.append("-DSTATIC_QT=ON")

        if triplet == "x64-windows-static":
            cmake_args.append("-DOPAQ_STATIC_RUNTIME=ON")
        elif triplet.startswith("x64-osx"):
            cmake_args.append("-DVCPKG_ALLOW_SYSTEM_LIBS=ON")

        if args.build_dist:
            vcpkg.build_project_release(
                "opaq", os.path.abspath(args.source_dir), triplet=triplet, cmake_args=cmake_args
            )
        else:
            vcpkg.build_project("opaq", os.path.abspath(args.source_dir), triplet=triplet, cmake_args=cmake_args)
    except KeyboardInterrupt:
        print("\nInterrupted")
        sys.exit(-1)
    except RuntimeError as e:
        print(e)
        sys.exit(-1)
