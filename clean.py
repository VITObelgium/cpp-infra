#!/usr/bin/env python3
import argparse
import subprocess
import sys
import os
import shutil
import sysconfig


def vcpkg_root_dir():
    return os.path.abspath(os.path.join(os.path.dirname(__file__)))


def run_vcpkg(triplet, vcpkg_args):
    if not shutil.which("vcpkg"):
        raise RuntimeError("vcpkg executable not found in the PATH environment")

    args = ["vcpkg", "--vcpkg-root", vcpkg_root_dir()]
    if triplet:
        args += ["--triplet", triplet]
    args += vcpkg_args

    subprocess.check_call(args)


def run_vcpkg_output(triplet, vcpkg_args):
    if not shutil.which("vcpkg"):
        raise RuntimeError("vcpkg executable not found in the PATH environment")

    args = ["vcpkg", "--vcpkg-root", vcpkg_root_dir()]
    if triplet:
        args += ["--triplet", triplet]
    args += vcpkg_args

    return subprocess.check_output(args).decode("UTF-8")


def vcpkg_list_ports(triplet):
    args = ["list"]
    ports = set()
    for line in run_vcpkg_output(triplet, args).splitlines():
        name, trip = tuple(line.split()[0].split(":"))
        if triplet is None or trip == triplet:
            if not "[" in name:
                ports.add(name)

    return ports


def clean(triplet, all):
    if triplet is None:
        shutil.rmtree(os.path.join(vcpkg_root_dir(), "installed"))
        shutil.rmtree(os.path.join(vcpkg_root_dir(), "buildtrees"))
        shutil.rmtree(os.path.join(vcpkg_root_dir(), "packages"))
        return

    for directory in os.listdir(os.path.join(vcpkg_root_dir(), "packages")):
        package, package_triplet = tuple(directory.split("_"))
        if package.startswith("."):
            continue
        if package_triplet == triplet:
            shutil.rmtree(os.path.join(vcpkg_root_dir(), "packages", directory))

    ports = vcpkg_list_ports(triplet)
    if len(ports) > 0:
        run_vcpkg(triplet, ["remove", "--recurse"] + list(ports))


if __name__ == "__main__":
    try:
        parser = argparse.ArgumentParser(description="Bootstrap vcpkg ports.")
        parser.add_argument(
            "-t",
            "--triplet",
            dest="triplet",
            metavar="TRIPLET",
            help="the triplet to use",
        )
        parser.add_argument(
            "-a", "--all", dest="all", help="also delete the installed directory"
        )
        args = parser.parse_args()
        clean(args.triplet, args.all)
    except KeyboardInterrupt:
        print("Interrupted")
        sys.exit(-1)
    except RuntimeError as e:
        print(e)
        sys.exit(-1)
