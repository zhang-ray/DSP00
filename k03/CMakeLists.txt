project(k03)

cmake_minimum_required(VERSION 3.4.1)

#add_definitions(-DANDROID_ARM_NEON=TRUE -DANDROID_ABI=armeabi-v7a -DCMAKE_BUILD_TYPE=Release )

add_subdirectory(./kfr)

include_directories(. ./kfr/include /home/administrator/work/eigen)

#find_library(log-lib log )

add_executable(k03 Entry.cpp)
#add_library(feature3 SHARED JNI.cpp)
target_link_libraries(k03 kfr kfr_dft kfr_io use_arch portaudio)
