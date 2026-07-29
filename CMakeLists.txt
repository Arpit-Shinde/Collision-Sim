cmake_minimum_required(VERSION 3.10)
project(Collision_Sim VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Find dependencies using vcpkg
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

# 2. Add your source files
add_executable(Collision_Sim 
    app/main.cpp 
    src/engine.cpp
    src/geometry.cpp
    src/shader.cpp
    src/glad.c
)

# 3. Include headers and link libraries
target_include_directories(Collision_Sim PRIVATE include)
target_link_libraries(Collision_Sim PRIVATE glfw OpenGL::GL)

# 4. Installer Configuration (CPack)
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "CollisionSim")
set(CPACK_PACKAGE_VENDOR "YourName")
set(CPACK_PACKAGE_VERSION "1.0.0")

if(WIN32)
    set(CPACK_GENERATOR "WIX") # Generates an MSI installer
endif()

install(TARGETS Collision_Sim DESTINATION bin)
include(CPack)