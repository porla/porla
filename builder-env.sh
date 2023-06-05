#!/bin/bash -v
architecture=$(uname -m)

if [[ $architecture == "x86_64" ]]; then
    binary_url="https://github.com/userdocs/ninja-build-static/releases/download/v1.11.1/ninja-amd64"
    cmake_options="-DCMAKE_BUILD_TYPE=Release -DVCPKG_TARGET_TRIPLET=x64-linux-musl-release"
elif [[ $architecture == "aarch64" ]]; then
    binary_url="https://github.com/userdocs/ninja-build-static/releases/download/v1.11.1/ninja-arm64v8"
    cmake_options='-DCMAKE_BUILD_TYPE=Release -DVCPKG_TARGET_TRIPLET=arm64-linux-musl-release'
else
   echo "Unsupported architecture: $architecture"
   exit 1
fi

echo "Downloading binary for $architecture..."
wget "$binary_url" -O ninja
echo "Binary downloaded successfully"
mv ninja /usr/bin
chmod +x /usr/bin/ninja

echo "Building for $architecture..."
mkdir build && cd build
cmake -DCMAKE_EXE_LINKER_FLAGS="-static -Os" $cmake_options -G Ninja ..
