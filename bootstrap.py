#!/usr/bin/env python3
import os
import sys
import sysconfig
import argparse

from deps.vcpkg.scripts.buildtools import vcpkg

if __name__ == "__main__":
    try:
        parser = argparse.ArgumentParser(
            description="Bootstrap infra.", parents=[vcpkg.bootstrap_argparser()]
        )

        parser.add_argument(
            "--cpp20",
            dest="cpp20_enabled",
            action="store_true",
            help="build with C++20 support",
        )

        args = parser.parse_args()

        platform = sysconfig.get_platform()
        triplet = args.triplet
        if platform == "win-amd64":
            triplet = "x64-windows-static-vs2022"
        elif platform == "mingw":
            triplet = "x64-mingw"
        elif not triplet:
            triplet = vcpkg.prompt_for_triplet()

        extras = []
        features = [
            "log",
            "cliprogress",
            "process",
            "hashing",
            "xml",
            "numeric",
            "charset",
            "compression",
            "gdal",
            "db",
            "testing",
        ]

        if not args.cpp20_enabled:
            features.append("cpp17")

        build_root = None
        packages_root = None

        if args.clean:
            vcpkg.clean(triplet=triplet)
        else:
            vcpkg.bootstrap(
                ports_dir=os.path.join(".", "deps"),
                triplet=triplet,
                additional_ports=extras,
                build_root=build_root,
                packages_root=packages_root,
                additional_features=features,
            )
    except KeyboardInterrupt:
        print("\nInterrupted")
        sys.exit(-1)
    except RuntimeError as e:
        print(e)
        sys.exit(-1)
