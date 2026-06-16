{ pkgs, ... }:

{
  env.VCPKG_FORCE_DOWNLOADED_BINARIES = "true";

  # https://devenv.sh/packages/
  packages = with pkgs; [
    cmake
    ninja
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
