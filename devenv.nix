{ pkgs, lib, config, inputs, ... }:

{
  packages = [
    pkgs.boost
    pkgs.cmake
    pkgs.libgit2
    pkgs.libsodium
    pkgs.libtorrent-rasterbar
    pkgs.lua
    pkgs.ninja
    pkgs.openssl
    pkgs.sqlite
    pkgs.zlib
  ];
}
