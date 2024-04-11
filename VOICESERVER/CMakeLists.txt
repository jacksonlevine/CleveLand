cmake_minimum_required(VERSION 3.10)

# set the project name
project(hungaryserver)

add_executable(main src/main.cpp)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

find_package(portaudio CONFIG REQUIRED)
target_link_libraries(main PRIVATE portaudio_static)

find_package(Opus CONFIG REQUIRED)
target_link_libraries(main PRIVATE Opus::opus)

find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(main PRIVATE Boost::boost Boost::system)