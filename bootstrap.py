#!/usr/bin/env python3
import os
import sys
import platform
import argparse

sys.path.insert(0, os.path.join(".", "deps", "vcpkg", "scripts"))
from buildtools import vcpkg

if __name__ == "__main__":
    try:
        parser = argparse.ArgumentParser(description="Bootstrap opaq.")
        parser.add_argument("--ui", dest="ui_enabled", action="store_true", help="build the ui")
        parser.add_argument(
            "--static", dest="static_runtime", default=False, action="store_true", help="Use the static runtime (windows only)"
        )
        parser.add_argument(
            "--upgrade", dest="upgrade", default=False, action="store_true", help="Upgrade installed packages"
        )
        args = parser.parse_args()

        triplet = "x64-windows" if platform.system() == "Windows" else None
        if triplet and args.static_runtime:
            triplet += "-static"

        extras = []
        if args.ui_enabled:
            extras.extend(["qt5[qml,tools,location,sql]", "gdal", "gsl"])

        if args.upgrade:
            vcpkg.upgrade(triplet=triplet)
        else:
            vcpkg.bootstrap(ports_dir=os.path.join(".", "deps"), triplet=triplet, additional_ports=extras)
    except KeyboardInterrupt:
        print("\nInterrupted")
        sys.exit(-1)
    except RuntimeError as e:
        print(e)
        sys.exit(-1)
