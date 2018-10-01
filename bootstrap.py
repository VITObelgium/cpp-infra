#!/usr/bin/env python3
import os
import sys
import platform

sys.path.insert(0, os.path.join(".", "deps", "vcpkg", "scripts"))
from buildtools import vcpkg

if __name__ == "__main__":
    try:
        triplet = "x64-windows" if platform.system() == "Windows" else None
        if len(sys.argv) == 2 and sys.argv[1] == "upgrade":
            vcpkg.upgrade(triplet=triplet)
        else:
            vcpkg.bootstrap(ports_dir=os.path.join(".", "deps"), triplet=triplet)
    except KeyboardInterrupt:
        print("\nInterrupted")
        sys.exit(-1)
    except RuntimeError as e:
        print(e)
        sys.exit(-1)
