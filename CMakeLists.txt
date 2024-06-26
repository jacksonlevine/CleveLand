cmake_minimum_required(VERSION 3.10)

# set the project name
project(cleveland)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(main ${SOURCES})

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
endif()

target_include_directories(main PUBLIC ${CMAKE_SOURCE_DIR}/include)


find_package(Boost REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(main PRIVATE Boost::boost Boost::system)



find_package(portaudio CONFIG REQUIRED)
target_link_libraries(main PRIVATE portaudio_static)
find_package(SndFile CONFIG REQUIRED)
target_link_libraries(main PRIVATE SndFile::sndfile)

find_package(EnTT CONFIG REQUIRED)
target_link_libraries(main PRIVATE EnTT::EnTT)
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(main PRIVATE glfw)
find_package(glm CONFIG REQUIRED)
target_link_libraries(main PRIVATE glm::glm)
find_package(GLEW REQUIRED)
target_link_libraries(main PRIVATE GLEW::GLEW)
