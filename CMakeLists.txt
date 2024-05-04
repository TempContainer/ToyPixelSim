cmake_minimum_required(VERSION 3.18)
project(ToyPixelSim VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_subdirectory(glad)
add_executable(sim src/main.cpp)
target_link_libraries(sim PUBLIC glad glfw glm::glm)
