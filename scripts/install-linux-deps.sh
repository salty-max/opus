#!/usr/bin/env bash
# System packages needed to build Opus on Debian/Ubuntu: Ninja, pkg-config,
# and the development headers SDL3 compiles its Linux backends against
# (X11, Wayland, audio, GL/EGL, input). SDL loads these libraries at run
# time, so games need only the runtime packages, not these.
#
# Used by CI and the release workflow; run it once on a new Linux machine.
set -euo pipefail

packages=(
  ninja-build pkg-config
  # windowing
  libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev
  libxss-dev libxtst-dev libxkbcommon-dev libwayland-dev libdecor-0-dev
  # rendering
  libegl1-mesa-dev libgl1-mesa-dev libgles2-mesa-dev libdrm-dev libgbm-dev
  # audio
  libasound2-dev libpulse-dev libpipewire-0.3-dev
  # input and desktop integration
  libudev-dev libdbus-1-dev libibus-1.0-dev
)

sudo apt-get update -qq
sudo apt-get install -y --no-install-recommends "${packages[@]}"
