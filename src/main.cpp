// g++ -std=c++23 src/main.cpp src/glad.c -o src/main.exe -Iinclude -Llib -lglfw3 -lopengl32 -lgdi32 ; src/main.exe (ignore this command)

/*

Version 2.0

Description : 
    -Basic implementation of particle collision simulation
    -Naive O(n²) time complexity for collision detection

Improvements made:
    -Switched to instanced rendering (1 draw call for all particles)
     
    
Changes:
    -updated vertex shader to recieve pos, color and offset
    -updated fragment shader to recieve color from vertex shader (abandoned uniform)
    -Added wall collisions 
    -removed camera system (panning and zooming)
    
*/

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <shaders/shader.h>
#include <engine.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <bits/stdc++.h>
#include <random>



int resolution = 15; //resolution for the circle. more res, more round.
int n = 20; //number of particles = n*n.
float radius =0.03f;
const float restitution = 1.0f;

void processinput(GLFWwindow* window);//process user input

int main() {
    GLFWwindow* window = init_window();

    unsigned int shaderProgram = linkShaders();

    //generate Base Circle Vertices (Centered at 0,0,0)
    std::vector<float> vertices;
    
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(1.0f); //color of the particles (center)
    vertices.push_back(1.0f);
    vertices.push_back(1.0f);

    for (int i = 0; i <= resolution; i++) {
        float angle = i * 2.0f * std::numbers::pi_v<float> / resolution;

        vertices.push_back(radius* std::cos(angle));
        vertices.push_back(radius* std::sin(angle));
        vertices.push_back(0.0f);
        vertices.push_back(1.0f); //color of the particles
        vertices.push_back(1.0f);
        vertices.push_back(1.0f);
    }

    // Create the grid once at the start
    glm::vec2 positions[n*n];
    
    float spacing = 0.1f;
    int index = 0;
    
    for (int y = 0; y < n; y += 1) {
        for (int x = 0; x < n; x += 1) {
            float x_pos = -1+ 2*(x/(float)n);
            float y_pos = -1+ 2*(y/(float)n);
            positions[index++] = glm::vec3(x_pos, y_pos,0);
        }
    }

    
    unsigned int instanceVBO;
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    //Use GL_DYNAMIC_DRAW because we will update positions every frame
    glBufferData(GL_ARRAY_BUFFER,sizeof(glm::vec2)*n*n, &positions[0], GL_STATIC_DRAW); 
    glBindBuffer(GL_ARRAY_BUFFER,0);


    
    unsigned int VAO, geometryVBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &geometryVBO);
    glBindBuffer(GL_ARRAY_BUFFER, geometryVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));

    
    
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(2, 1);
    glBindVertexArray(0);




    //random velocities
    std::vector <glm::vec2> velocities;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution <float> random_vel(-0.2f,0.2f); //set the initial random velocities here

    for (int i = 0;i<n*n;i++){
        velocities.push_back(glm::vec2(random_vel(gen), random_vel(gen)));
    }

    
    float prev_time = 0;
    double lastTime = glfwGetTime();
    int frames = 0;
    int count=0;
    int fps=0;


    while (!glfwWindowShouldClose(window)){

        if (count==60) break;

        processinput(window);

        float time = glfwGetTime();
        float dt = time - prev_time;
        prev_time = time;

        dt = glm::min(dt, 0.01f); //smaller cap for stability

        frames++;

        if (time - lastTime >= 1.0){
            std::cout << "FPS: " << frames << std::endl;
            fps+=frames;
            frames = 0;
            lastTime = time;
            count++;
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        //check collision

        for (int i = 0; i < n*n; i++) {

            // Lower wall (y = -1)
            if (positions[i].y - radius < -1.0f) {
                positions[i].y = -1.0f + radius;  // Snap to surface
                velocities[i].y = -velocities[i].y * restitution;
            }

            // Upper wall (y = 1)
            if (positions[i].y + radius > 1.0f) {
                positions[i].y = 1.0f - radius;   // Snap to surface
                velocities[i].y = -velocities[i].y * restitution;
            }

            // Left wall (x = -1)
            if (positions[i].x - radius < -1.0f) {
                positions[i].x = -1.0f + radius;  // Snap to surface
                velocities[i].x = -velocities[i].x * restitution;
            }

            // Right wall (x = 1)
            if (positions[i].x + radius > 1.0f) {
                positions[i].x = 1.0f - radius;   // Snap to surface
                velocities[i].x = -velocities[i].x * restitution;
            }
            //check collision with other particles
            for (int j = i + 1; j < n*n; j++) {

                glm::vec2 delta = positions[i] - positions[j];

                float dist = glm::length(delta);
                
                float minDist = 2*radius; //assuming simulation for identical bodies
                

                if (dist < minDist && dist > 0.00001f) { //the later condition is to avoid normalizing a zero length vector 
                    
                    glm::vec2 n = glm::normalize(delta);

                    float v1n = glm::dot(velocities[i], n);
                    float v2n = glm::dot(velocities[j], n);

                    glm::vec2 v1t = velocities[i] - v1n * n;
                    glm::vec2 v2t = velocities[j] - v2n * n;

                    // Exchange the normal components
                    velocities[i] = v1t + (v1n + (1 + restitution) * (v2n - v1n) * 0.5f) * n;
                    velocities[j] = v2t + (v2n + (1 + restitution) * (v1n - v2n) * 0.5f) * n;

                    float penetration = minDist - dist; 
                    

                    positions[i] += n * (penetration*0.5f);//in case of overlap, seperate them completely 
                    positions[j] -= n * (penetration*0.5f);
                }
            }
        }

        for (int i = 0;i<n*n;i++){
            positions[i] +=velocities[i]*dt;
            
        }

        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

        glBufferSubData(
            GL_ARRAY_BUFFER,
            0,
            sizeof(glm::vec2) * n*n,
            positions
        );

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(VAO);
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, resolution+2, n*n);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    float avg = fps/(float) count;
    std::cout<<"\n---------\navg fps = "<<avg<<"\n"; 

    
    glfwTerminate();


    return 0;
}


void processinput(GLFWwindow* window){
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }
}
