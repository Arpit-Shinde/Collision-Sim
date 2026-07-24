# Collision Simulator
An **OpenGL** based GPU accelerated 2D particle (circles) simulator which simulates real-time collisions amongst identical particles. 


##  Features
*   **Performance Metrics:** Real-time FPS counter displayed in the console, with average fps over a minute displayed at the end.
*   **Configurable Simulation Parameters:** Allows user to configure number of particles, radius of particles, coefficient of restitution and the resolution of particle.

##  Technical Details
*   **Collision Detection:** Implemented using spatial grid having roughly $O(n)$ time complexity for uniform distribution.
*   **Integration:** Euler integration.
*   **Rendering:** Instanced rendering using `glDrawArraysInstanced`. (Single draw call for all particles)
*   **Initialisation:** Particles are initialised on a grid with random velocities (adjustable range)
*   **GPGPU** Implemented Compute shaders for GPU parallelism 

##  Architecture Document
To understand the architecture of this project, read the  **[Architecture Document](https://arpit-shinde.github.io/Collision-Sim/)**.


##  How to Build
Click the `code` button (green coloured) and copy the HTTPS URL. Open a folder in VS Code and run the command

```bash
git clone https://github.com/Arpit-Shinde/Collision-Sim.git
```
This will clone this repository. Once done, open the folder `Collision-Sim` in VS Code (Ctrl+K+O) and run the command

```bash
g++ -std=c++23 src/main.cpp src/glad.c -o src/main.exe -Iinclude -Llib -lglfw3 -lopengl32 -lgdi32 ; src/main.exe
``` 
in the terminal. Make sure the terminal is open in the folder `Collision-Sim` only. Build commands supported for Windows only.