# Collision Simulator
An **OpenGL** based GPU accelerated 2D particle (circles) simulator which simulates real-time collisions amongst identical particles. 


##  Features
*   **Performance Metrics:** Real-time FPS counter displayed in the console, with average fps over a minute displayed at the end.
*   **Color Coding:** Particles are mapped to a color based on their speed
*   **Configurable Simulation Parameters:** Allows user to configure number of particles, radius of particles, coefficient of restitution and the resolution of particle.

*Note : The data can be configured in* `app/main.cpp`

##  Technical Details
*   **Collision Detection:** Implemented using spatial grid having roughly $O(n)$ time complexity for uniform distribution.
*   **Integration:** Euler integration.
*   **Rendering:** Instanced rendering using `glDrawArraysInstanced`. (Single draw call for all particles)
*   **Initialisation:** Particles are initialised on a grid with random velocities (adjustable range)
*   **GPGPU** Implemented Compute shaders for GPU parallelism 
*   **Build System** Configured with CMake to handle dependency management across environments.
*   **GPU Optimisation** To make use of dedicated graphics card, `src/gpu_config.cpp` must be added to `add_executable` list in `CMakeLists.txt`.

##  Architecture Document
To understand the architecture of this project, read the  **[Architecture Document](https://arpit-shinde.github.io/Collision-Sim/)**.


## How to Build

Click the green `Code` button and copy the HTTPS URL. Open a folder in VS Code and run the following command in the terminal (make sure to have git installed):

```bash
git clone https://github.com/Arpit-Shinde/Collision-Sim.git
```

This will clone the repository. Once done, open the `Collision-Sim` folder in VS Code (`Ctrl+K+O`).

### Installing CMake

Run the following command (make sure your terminal is open in the `Collision-Sim` directory):

```bash
winget install Kitware.CMake
```

*Note: Reopen VS Code after running this command*

### Compiling the Project

Now, open the terminal in the working directory and run the following command to create a build folder:

```bash
mkdir build
```

Navigate to the newly created build folder:

```bash
cd build
```

Configure the project by running:

```bash
cmake .. -G "MinGW Makefiles"
```

After completion, build the executable with:

```bash
cmake --build .
```
### Running the Project
Once the build is complete, you will find the executable `Collsion_Sim.exe` inside your build directory. You can launch it by double-clicking the file, or run it directly from your terminal.



