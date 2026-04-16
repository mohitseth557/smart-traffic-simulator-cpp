cmake_minimum_required(VERSION 3.14)
project(SmartTrafficSim VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find raylib
find_package(raylib QUIET)
if(NOT raylib_FOUND)
    # Fallback: try Homebrew paths
    set(RAYLIB_INC "/opt/homebrew/include" CACHE PATH "")
    set(RAYLIB_LIB "/opt/homebrew/lib"     CACHE PATH "")
    include_directories(${RAYLIB_INC})
    link_directories(${RAYLIB_LIB})
endif()

set(SOURCES
    src/main.cpp
    src/vehicle.cpp
    src/controller.cpp
    src/simulation.cpp
    src/renderer.cpp
)

add_executable(SmartTrafficSim ${SOURCES})
target_include_directories(SmartTrafficSim PRIVATE include)

if(APPLE)
    target_link_libraries(SmartTrafficSim raylib
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
else()
    target_link_libraries(SmartTrafficSim raylib)
endif()
