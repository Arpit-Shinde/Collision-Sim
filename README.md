# Particle Collision Simulator

A lightweight 2D particle physics simulation built using **C++** and **OpenGL**. This project demonstrates fundamental concepts in physics integration, collision detection, and real-time graphics rendering.

## 🚀 Features
*   **Physics Simulation:** Real-time elastic collision resolution between particles.
*   **Camera System:** Intuitive panning and zooming controls to explore the simulation space.
*   **Visuals:** Dynamic rendering of circles using `GL_TRIANGLE_FAN` with configurable resolution.
*   **Performance Metrics:** Real-time FPS counter displayed in the console.

## 🛠 Technical Details
*   **Collision Detection:** Implemented using a naive $O(n^2)$ algorithm.
*   **Integration:** Euler integration (with a placeholder for future Verlet integration support).
*   **Rendering:** Individual draw calls per particle using `glDrawArrays`.

## 🎮 Controls
| Action | Key |
| :--- | :--- |
| **Move (Pan)** | Arrow Keys |
| **Zoom (In/Out)** | W / S |
| **Exit** | ESC |

## ⚙️ How to Build
This project uses **GLFW** and **GLAD**. Ensure your environment is configured with these libraries. To compile using `g++` on Windows:

```bash
g++ -std=c++23 src/main.cpp src/glad.c -o src/main.exe -Iinclude -Llib -lglfw3 -lopengl32 -lgdi32