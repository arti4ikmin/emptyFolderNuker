cmake_minimum_required(VERSION 3.31)
project(emptyFolderNuker)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(emptyFolderNuker main.cpp)

install(TARGETS emptyFolderNuker DESTINATION bin)