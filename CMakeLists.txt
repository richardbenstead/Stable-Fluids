cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O1")
set(CMAKE_CXX_FLAGS_RELEASE "-g -O3 -DNDEBUG")

project(Stable-Fluids)
add_executable(Stable-Fluids src/main.cpp)
target_link_libraries(Stable-Fluids PUBLIC fftw3f GL glfw3 dl pthread)
