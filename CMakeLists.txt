cmake_minimum_required(VERSION 3.10)

project(Lang)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

include_directories(
  include
)

add_executable(lang src/main.cpp
                    src/parsing_tree.cpp
                    src/type_check.cpp
                    src/mode_check.cpp)
