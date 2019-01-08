#!/usr/bin/env python3
import os
import sys
import platform

sys.path.insert(0, os.path.join(".", "deps", "vcpkg", "scripts"))
from buildtools import vcpkg

if __name__ == "__main__":
    try:
        build_ui = "ON" if platform.system() == "Windows" else "OFF"
        triplet = "x64-windows-static" if platform.system() == "Windows" else None
        if triplet is None:
            triplet = vcpkg.prompt_for_triplet()

        generator = "Xcode" if triplet == "x64-osx" else None

        cmake_args = ["-DINFRA_EMBED_GDAL_DATA=ON"]
        if triplet == "x64-wasm":
            cmake_args.extend([
                "-DGDX_DISABLE_OPENMP=ON",
                "-DGDX_ENABLE_TOOLS=OFF",
                "-DGDX_ENABLE_TESTS=OFF",
                "-DGDX_PYTHON_BINDINGS=OFF",
                "-DINFRA_ENABLE_TESTS=OFF",
            ])

        if len(sys.argv) == 2 and sys.argv[1] == "dist":
            vcpkg.build_project_release(
                "gdx", os.path.abspath("."), triplet=triplet, cmake_args=cmake_args
            )
        else:
            vcpkg.build_project(
                "gdx",
                os.path.abspath("."),
                triplet=triplet,
                cmake_args=cmake_args,
                generator=generator,
            )
    except KeyboardInterrupt:
        print("\nInterrupted")
        sys.exit(-1)
    except RuntimeError as e:
        print(e)
        sys.exit(-1)
