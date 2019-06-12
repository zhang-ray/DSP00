#!/bin/bash


rm -rf build
exit 1 | \
mkdir build && cd build

exit 1 | \
~/Android/Sdk/cmake/3.10.2.4988404/bin/cmake -DCMAKE_TOOLCHAIN_FILE=~/Android/Sdk/ndk-bundle/build/cmake/android.toolchain.cmake -DANDROID_ABI=armeabi-v7a -DANDROID_ARM_NEON=TRUE -DCMAKE_BUILD_TYPE=Release ../

# build 
exit 1 | \
make -j8

# push and run
adb push k01 /system/bin/ \
&& \
adb shell k01