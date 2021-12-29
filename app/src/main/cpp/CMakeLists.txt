cmake_minimum_required(VERSION 3.10.2)

set(CMAKE_CXX_STANDARD 11)

project("scut")

set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/../jniLibs)

include_directories(${INCLUDE_DIR})
link_directories(${LIB_DIR}/${ANDROID_ABI})

aux_source_directory(. src)

add_library(srp SHARED ${src})

#find_library(native-lib log)

find_package(oboe REQUIRED CONFIG)

set(third-party sox snore)

set(native-lib log omp)

target_link_libraries(srp ${native-lib} ${third-party} oboe::oboe)