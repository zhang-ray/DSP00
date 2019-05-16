#!/bin/bash


rm -rf build
mkdir build && cd build


~/Android/Sdk/cmake/3.10.2.4988404/bin/cmake -DCMAKE_TOOLCHAIN_FILE=~/Android/Sdk/ndk-bundle/build/cmake/android.toolchain.cmake -DANDROID_ABI=armeabi-v7a -DANDROID_ARM_NEON=TRUE -DCMAKE_BUILD_TYPE=Release ../

# build 
make -j8

# push and run
adb push k01 /system/bin/ \
&& \
adb shell k01