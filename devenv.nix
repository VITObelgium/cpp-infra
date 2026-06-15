{ pkgs, ... }:

{
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
    python314
  ];
}
