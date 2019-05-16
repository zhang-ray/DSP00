project(k01)

cmake_minimum_required(VERSION 3.4.1)

# reference:
#~/Android/Sdk/cmake/3.10.2.4988404/bin/cmake  \
#-GNinja -DENABLE_TESTS=ON ../kfr\
# -DCMAKE_TOOLCHAIN_FILE=~/Android/Sdk/ndk-bundle/build/cmake/android.toolchain.cmake \
#-DANDROID_ABI=armeabi-v7a \
#-DANDROID_ARM_NEON=TRUE -DSKIP_TESTS=ON \
#-DCMAKE_BUILD_TYPE=Release


add_definitions(-DANDROID_ARM_NEON=TRUE -DANDROID_ABI=armeabi-v7a -DCMAKE_BUILD_TYPE=Release )

add_subdirectory(./kfr)

include_directories(. ./kfr/include)

find_library(log-lib log )


add_executable(k01 K01.cpp)
target_link_libraries(k01 ${log-lib} kfr kfr_dft kfr_io use_arch)
