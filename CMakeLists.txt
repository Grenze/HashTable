cmake_minimum_required(VERSION 3.12)
project(CuckooHash)

set(CMAKE_CXX_STANDARD 17)

add_executable(CuckooHash main.cpp BitsUtil.h HashUtil.cpp HashUtil.h PrintUtil.h BaseTable.h HashTable.h)