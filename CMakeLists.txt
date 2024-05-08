cmake_minimum_required(VERSION 3.10)

project(Evaluation)

find_package(fmt CONFIG REQUIRED)

add_executable(evaluation src/main.cpp src/trial.cpp)

target_link_libraries(evaluation PRIVATE fmt::fmt)