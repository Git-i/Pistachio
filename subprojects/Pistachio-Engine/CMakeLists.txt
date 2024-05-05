cmake_minimum_required(VERSION 3.7...3.29)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(Pistachio LANGUAGES CXX)
add_library(Pistachio SHARED)
target_compile_features(Pistachio cxx_std_17)
