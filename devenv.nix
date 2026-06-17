{ pkgs, ... }:

{
  env.VCPKG_FORCE_DOWNLOADED_BINARIES = "true";
  env.VCPKG_ROOT = "${pkgs.vcpkg}/share/vcpkg";

  # https://devenv.sh/packages/
  packages = with pkgs; [
    cmake
    ninja
    vcpkg
    nasm
    just
    pkg-config
    automake
    autoconf
    autoconf-archive
    flex
    bison
    libtool
    clang-tools
    python314
  ];
}
