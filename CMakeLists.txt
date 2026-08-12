cmake_minimum_required(VERSION 3.10)
project(Collision_Sim)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

link_directories(lib)

add_executable(Collision_Sim 
    app/main.cpp 
    src/engine.cpp
    src/geometry.cpp
    src/shader.cpp
    src/glad.c
    # src/gpu_config.cpp # keep it as it is if testing on integrated graphics card is required else uncomment this whole line if testing on dedicated graphics card is required
)

# need opengl32 for Windows, and glfw3 for GLFW
target_link_libraries(Collision_Sim 
    glfw3 
    opengl32
    gdi32
)
# uncomment the below section for creating a standalone .exe file 
# install(TARGETS Collision_Sim RUNTIME DESTINATION bin)

# # --- CPack ZIP Configuration ---
# set(CPACK_GENERATOR "ZIP")
# set(CPACK_PACKAGE_NAME "CollisionSim")
# set(CPACK_PACKAGE_VERSION "1.0.0")

# include(CPack)

