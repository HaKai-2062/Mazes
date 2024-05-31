# From: https://github.com/meemknight/cmakeSetup/blob/master/thirdparty/glad/CMakeLists.txt

cmake_minimum_required(VERSION 3.5)
project(glad)

add_library(glad)
target_sources(glad PRIVATE 
"${CMAKE_CURRENT_SOURCE_DIR}/../../Glad/src/glad.c"
)
target_include_directories(glad PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../Glad/include")
