# detect the vcpkg triplet based on the system information
windows_triplet := "x64-windows-static"
VCPKG_DEFAULT_TRIPLET := if os_family() == "windows" {
  windows_triplet
  } else if os() == "macos" {
    if arch() == "aarch64" {
      "arm64-osx"
    } else { "x64-osx" }
  } else {
    "x64-linux"
  }

cmake_preset := if os_family() == "windows" {
    "windows"
} else if os() == "macos" {
    if arch() == "aarch64" {
        "mac-arm"
    } else { "mac-intel" }
} else {
    "linux"
}

VCPKG_DEFAULT_HOST_TRIPLET := VCPKG_DEFAULT_TRIPLET
vcpkg_root := env('VCPKG_ROOT', join(justfile_directory(), "..", "vcpkg"))

export VCPKG_OVERLAY_TRIPLETS := join(source_directory(), "vcpkg_overlay", "triplets")
export VCPKG_OVERLAY_PORTS := join(source_directory(), "vcpkg_overlay", "ports")

git_status_clean:
    git diff --quiet --exit-code

# will invoke just build_dist but first activates the visual studio environment
[windows]
dist:
    cmd.exe /C '{{source_directory()}}/dist.bat' '{{justfile_directory()}}'

[unix]
dist: build_dist