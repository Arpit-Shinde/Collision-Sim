cmake_minimum_required(VERSION 3.10)
project(Collision_Sim VERSION 1.0.0 LANGUAGES CXX)

# Standard C++ settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 1. Find Dependencies (vcpkg handles these automatically)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

# 2. Define the executable
add_executable(Collision_Sim 
    app/main.cpp 
    src/engine.cpp
    src/geometry.cpp
    src/shader.cpp
    src/glad.c
)

# 3. Include and Link
# Using modern CMake targets (the '::' syntax)
target_include_directories(Collision_Sim PRIVATE include)
target_link_libraries(Collision_Sim PRIVATE glfw OpenGL::GL)

# 4. Packaging / Installer Configuration (CPack)
# This section tells CPack how to create your installer
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "Collision_Sim")
set(CPACK_PACKAGE_VENDOR "Arpit Shinde")
set(CPACK_PACKAGE_CONTACT "arpitshinde9@gmail.com")
set(CPACK_PACKAGE_VERSION_MAJOR 1)
set(CPACK_PACKAGE_VERSION_MINOR 0)
set(CPACK_PACKAGE_VERSION_PATCH 0)

# On Windows, generate an NSIS installer (.exe installer)
if(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_MODIFY_PATH ON)
else()
    # On Linux, generate a generic tarball
    set(CPACK_GENERATOR "TGZ")
endif()

# Tell CPack where the executable goes in the installed folder
install(TARGETS Collision_Sim DESTINATION bin)

# This includes everything necessary to run CPack
include(CPack)