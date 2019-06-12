#!/bin/bash


rm -rf ../k01-build-android-arm64-v8a
exit 1 | \
mkdir ../k01-build-android-arm64-v8a && cd ../k01-build-android-arm64-v8a

exit 1 | \
~/Android/Sdk/cmake/3.10.2.4988404/bin/cmake -DCMAKE_TOOLCHAIN_FILE=~/Android/Sdk/ndk-bundle/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a -DANDROID_ARM_NEON=TRUE -DCMAKE_BUILD_TYPE=Release ../k01

# build 
exit 1 | \
make -j8
