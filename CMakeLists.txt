cmake_minimum_required(VERSION 3.10)

# set the project name
project(cleveland)

file(GLOB_RECURSE SOURCES "src/*.cpp")



# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(main ${SOURCES}
        src/timetarget.h)

# MSVC specific configurations
if(MSVC)
    # Static runtime and optimization flags
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Os /MT /DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /D_CRT_SECURE_NO_WARNINGS /D_SCL_SECURE_NO_WARNINGS /D_SECURE_SCL=0")

    # Linker options to override entry, limit debugging symbols, and subsystem
    target_link_options(main PRIVATE 
        "/ENTRY:mainCRTStartup" 
        "/SAFESEH:NO" 
        "/NODEFAULTLIB:kernel32.lib" 
        "/OPT:REF" 
        # "/SUBSYSTEM:WINDOWS"
    )
endif()

# Include directories
target_include_directories(main PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Boost
find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(main PRIVATE Boost::boost Boost::system)

# Other libraries
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
