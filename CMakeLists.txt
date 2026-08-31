cmake_minimum_required(VERSION 3.10)
project(mini_autodiff_cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(demo src/main.cpp)