#!/usr/bin/env python3
import argparse
import subprocess
import sys
import os
import shutil
import sysconfig


def git_status_is_clean():
    return subprocess.call(["git", "diff", "--quiet"], shell=True) == 0


def git_revision_hash():
    return subprocess.check_output(["git", "rev-parse", "HEAD"], shell=True).decode("utf-8").rstrip("\r\n")


def vcpkg_root_dir():
    return os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))


def _create_vcpkg_command(triplet, vcpkg_args):
    if not shutil.which("vcpkg"):
        raise RuntimeError("vcpkg executable not found in the PATH environment")

    vcpkg_root = os.path.join(os.path.dirname(__file__), "..", "..")

    args = ["vcpkg", "--vcpkg-root", vcpkg_root]
    if triplet:
        args += ["--triplet", triplet]
    args += vcpkg_args

    return args


def run_vcpkg(triplet, vcpkg_args):
    subprocess.check_call(_create_vcpkg_command(triplet, vcpkg_args))


def run_vcpkg_output(triplet, vcpkg_args):
    return subprocess.check_output(_create_vcpkg_command(triplet, vcpkg_args)).decode("UTF-8")


def cmake_configure(source_dir, build_dir, cmake_args, triplet=None, toolchain=None, generator=None):
    if not shutil.which("cmake"):
        raise RuntimeError("cmake executable not found in the PATH environment")

    cwd = os.getcwd()
    os.chdir(build_dir)

    args = ["cmake", "--vcpkg-root", vcpkg_root_dir()]
    args.append("-G")
    if generator is not None:
        args.append(generator)
    elif sysconfig.get_platform() == "win-amd64":
        args.append("Visual Studio 15 2017 Win64")
    elif sysconfig.get_platform() == "win32":
        args.append("Visual Studio 15 2017")
    else:
        args.append("Ninja")
        # do not append build type for msvc builds, otherwise debug libraries are not found (multi-config build)
        args.append("-DCMAKE_BUILD_TYPE=Release")

    if triplet is not None:
        args.append("-DVCPKG_TARGET_TRIPLET={}".format(triplet))

    if toolchain is not None:
        args.append("-DCMAKE_TOOLCHAIN_FILE={}".format(toolchain))

    args.extend(cmake_args)
    args.append(source_dir)

    subprocess.check_call(args)
    os.chdir(cwd)


def cmake_build(build_dir, config=None, target=None):
    args = ["cmake", "--build", build_dir]
    if config is not None:
        args.extend(["--config", config])

    if target is not None:
        args.extend(["--target", target])
    subprocess.check_call(args)


def vcpkg_install_ports(triplet, ports):
    args = ["install", "--recurse"]
    args += ports
    run_vcpkg(triplet, args)


def vcpkg_upgrade_ports(triplet):
    args = ["upgrade", "--no-dry-run"]
    run_vcpkg(triplet, args)


def get_all_triplets():
    triplets = []
    triplet_dir = os.path.join(os.path.dirname(__file__), "..", "..", "triplets")
    for filename in os.listdir(triplet_dir):
        if filename.endswith(".cmake"):
            triplet_name = os.path.splitext(filename)[0]
            triplet_useable_on_platform = False
            platform = sysconfig.get_platform()
            if "wasm" in triplet_name:
                triplet_useable_on_platform = True
            elif platform.startswith("linux"):
                triplet_useable_on_platform = (
                    "linux" in triplet_name or "mingw" in triplet_name or "musl" in triplet_name
                )
            elif platform.startswith("macosx"):
                triplet_useable_on_platform = "osx" in triplet_name or "mingw" in triplet_name
            elif platform.startswith("win"):
                triplet_useable_on_platform = "windows" in triplet_name
            elif platform.startswith("mingw"):
                triplet_useable_on_platform = "mingw" in triplet_name

            if triplet_useable_on_platform:
                triplets.append(triplet_name)

    return triplets


def select_ports_file(ports_dir, triplet):
    """Select the best matching ports file for the selected triplet
    Will look for a ports-'triplet'.txt file in the specified directory
    If it is not present the ports in ports_dir/ports.txt will be used as fallback
    """
    port_file_candidates = [
        os.path.join(ports_dir, "ports-{}.txt".format(triplet)),
        os.path.join(ports_dir, "ports.txt"),
    ]

    for candidate in port_file_candidates:
        if os.path.exists(candidate):
            return candidate

    raise RuntimeError("No ports file found in '{}'".format(ports_dir))


def read_ports_from_ports_file(ports_file):
    ports_to_install = []
    with open(ports_file) as f:
        content = f.readlines()
        for line in content:
            line = line.strip()
            if not line.startswith("#"):
                ports_to_install.append(line)

    return ports_to_install


def prompt_for_triplet():
    triplets = get_all_triplets()
    index = 1
    displaymessage = "Select triplet to use:\n"
    for triplet in triplets:
        displaymessage += "{}: {}\n".format(index, triplet)
        index += 1
    displaymessage += "--> "

    try:
        triplet_index = int(input(displaymessage))
        if triplet_index < 1 or triplet_index > len(triplets):
            raise RuntimeError("Invalid triplet index selected")
        return triplets[triplet_index - 1]
    except ValueError:
        raise RuntimeError("Invalid triplet option selected")


def bootstrap_argparser():
    parser = argparse.ArgumentParser(description="Bootstrap vcpkg ports.", add_help=False)
    parser.add_argument("-t", "--triplet", dest="triplet", metavar="TRIPLET", help="the triplet to use")
    parser.add_argument(
        "-p",
        "--ports-dir",
        dest="ports_dir",
        metavar="PORTS_DIR",
        help="directory containing the ports file descriptions",
    )

    return parser


def bootstrap(ports_dir, triplet=None, additional_ports = []):
    if triplet is None:
        triplet = prompt_for_triplet()

    ports_file = select_ports_file(ports_dir, triplet)
    print("Using ports defined in: {}".format(os.path.abspath(ports_file)))
    ports_to_install = read_ports_from_ports_file(ports_file)
    ports_to_install.extend(additional_ports)

    try:
        vcpkg_install_ports(triplet, ports_to_install)
    except subprocess.CalledProcessError as e:
        raise RuntimeError("Bootstrap failed: {}".format(e))


def upgrade(triplet=None):
    if triplet is None:
        triplet = prompt_for_triplet()

    try:
        vcpkg_upgrade_ports(triplet)
    except subprocess.CalledProcessError as e:
        raise RuntimeError("Upgrade failed: {}".format(e))


def build_project(project_name, project_dir, triplet=None, cmake_args=[], build_dir=None, generator=None, target=None):
    if triplet is None:
        triplet = prompt_for_triplet()

    if not build_dir:
        build_dir = os.path.join(project_dir, "build", "{}-{}".format(project_name, triplet))
    os.makedirs(build_dir, exist_ok=True)

    vcpkg_root = vcpkg_root_dir()
    toolchain_file = os.path.abspath(os.path.join(vcpkg_root, "scripts", "buildsystems", "vcpkg.cmake"))

    try:
        cmake_configure(project_dir, build_dir, cmake_args, triplet, toolchain=toolchain_file, generator=generator)
        cmake_build(build_dir, config="Release", target=target)
    except subprocess.CalledProcessError as e:
        raise RuntimeError("Build failed: {}".format(e))


def build_project_release(project_name, project_dir, triplet=None, cmake_args=[]):
    if not git_status_is_clean():
        raise RuntimeError("Git status is not clean")

    build_dir = os.path.join(project_dir, "build", "{}-{}-dist".format(project_name, triplet))

    if os.path.exists(build_dir):
        shutil.rmtree(build_dir, ignore_errors=True)

    git_hash = git_revision_hash()
    cmake_args.append("-DPACKAGE_VERSION_COMMITHASH=" + git_hash)
    build_project(project_name, project_dir, triplet, cmake_args, build_dir)

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


def bootstrap_argparser():
    parser = argparse.ArgumentParser(description="Build all the third party project dependencies.", add_help=False)
    parser.add_argument("--clean", dest="clean", help="clean the vcpkg build directories")
    parser.add_argument("-t", "--triplet", dest="triplet", metavar="TRIPLET", help="the triplet to use")


def build_argparser():
    parser = argparse.ArgumentParser(description="Build the project using vcpkg dependencies.", add_help=False)
    parser.add_argument("-t", "--triplet", dest="triplet", metavar="TRIPLET", help="the triplet to use")
    parser.add_argument(
        "-s",
        "--source-dir",
        dest="source_dir",
        metavar="SOURCE_DIR",
        default=".",
        help="directory containing the sources to build",
    )

    return parser


if __name__ == "__main__":
    try:
        parser = argparse.ArgumentParser(description="Bootstrap vcpkg ports.")
        parser.add_argument("-t", "--triplet", dest="triplet", metavar="TRIPLET", help="the triplet to use")
        parser.add_argument(
            "-p",
            "--ports-dir",
            dest="ports_dir",
            metavar="PORTS_DIR",
            help="directory containing the ports file descriptions",
        )
        args = parser.parse_args()

        bootstrap(args.ports_dir, args.triplet)
    except KeyboardInterrupt:
        print("Interrupted")
        sys.exit(-1)
    except RuntimeError as e:
        print(e)
        sys.exit(-1)
