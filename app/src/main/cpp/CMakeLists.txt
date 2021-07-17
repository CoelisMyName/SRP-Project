cmake_minimum_required(VERSION 3.10.2)

project("scut")

set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/../jniLibs)

include_directories(${INCLUDE_DIR})
link_directories(${LIB_DIR}/${ANDROID_ABI})

add_library(utils-lib SHARED utils.c)

#find_library(native-lib log)

set(third-party sox)

set(native-lib log omp)

target_link_libraries(utils-lib ${native-lib} ${third-party})