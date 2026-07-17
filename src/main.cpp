// g++ -std=c++23 src/main.cpp src/glad.c -o src/main.exe -Iinclude -Llib -lglfw3 -lopengl32 -lgdi32 ; src/main.exe (ignore this command)

/*

Version 1.0

Description : 
    -Basic implementation of particle collision simulation
    -Naive O(n²) time complexity for collision detection
    -individual draw calls for each particles (n*n calls)
    -camera system for panning and zooming
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



int res = 15; //resolution for the circle. more res, more round.
int n = 20; //number of particles

void processinput(GLFWwindow* window, glm::vec3 &cameraPos, glm::vec3 &cameraTarget);//process user input for panning and zooming

struct Circle{
    float radius;
    glm::vec3 position;
    int resolution;
    std::vector <float> vertices;
    glm::vec4 color;
    unsigned int transLoc;
    unsigned int colorLoc;
    unsigned int viewLoc;
    unsigned int projLoc;
    unsigned int shaderProgram;


    Circle(float r, glm::vec3 v, int res, glm::vec4 col,unsigned int s ){
        radius = r;
        position = v;
        resolution = res;
        color = col;
        shaderProgram = s;
    };

    unsigned int create(){

        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);

        for (int i = 0; i <= resolution; i++) {
            float angle = i * 2.0f * std::numbers::pi_v<float> / resolution;

            vertices.push_back(radius* std::cos(angle));
            vertices.push_back(radius* std::sin(angle));
            vertices.push_back(0.0f);
        }

        unsigned int myVAO;
        glGenVertexArrays(1, &myVAO);
        glBindVertexArray(myVAO); 

        unsigned int myVBO;
        glGenBuffers(1, &myVBO);
        

        glBindBuffer(GL_ARRAY_BUFFER, myVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);

        transLoc = glGetUniformLocation(shaderProgram, "trans");
        colorLoc = glGetUniformLocation(shaderProgram, "in_color");
        viewLoc = glGetUniformLocation(shaderProgram, "view");
        projLoc = glGetUniformLocation(shaderProgram, "projection");
        return myVAO;
    }

    void draw(unsigned int vao,glm::mat4 trans, glm::mat4 view, glm::mat4 projection){
        glBindVertexArray(vao);
        glUniformMatrix4fv(transLoc, 1, GL_FALSE, value_ptr(trans));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, value_ptr(projection));
        glUniform4fv(colorLoc, 1, glm::value_ptr(color));
        glDrawArrays(GL_TRIANGLE_FAN, 0, resolution + 2);
    }
};

int main() {
    GLFWwindow* window = init_window();

    unsigned int shaderProgram = linkShaders();

    //init particles
    std::vector <Circle> particles (n*n, Circle(0.02f, glm::vec3(0.1f,0.0f,0.0f), res, glm::vec4(1.0,1.0,0.0,1.0), shaderProgram));

    int count = 0;

    float spacing = 0.10f;

    //initialise a grid arrangement of particles
    for (int y = 0; y < n; y++) {
        for (int x = 0; x < n; x++) {
            particles[count].position = glm::vec3(
                -1+ 2*(x/(float)n),
                -1+ 2*(y/(float)n),
                0.0f
            );
            count++;
        }
    }



    //init particle vaos
    std::vector <unsigned int> particle_vaos;

    for (auto& x : particles){
        particle_vaos.push_back(x.create());
    }




    //random velocities
    std::vector <glm::vec3> velocities;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution <float> random_vel(-0.9f,0.9f); //set the initial random velocities here

    for (int i = 0;i<n*n;i++){
        velocities.push_back(glm::vec3(random_vel(gen), random_vel(gen),0));
    }

    
    float prev_time = 0;
    double lastTime = glfwGetTime();
    int frames = 0;

    glm::vec3 cameraPos    = glm::vec3(0.0f, 0.0f, 2.5f); 
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); 
    glm::vec3 upVector     = glm::vec3(0.0f, 1.0f, 0.0f); 

    while (!glfwWindowShouldClose(window)){

        processinput(window, cameraPos, cameraTarget);

        float time = glfwGetTime();
        float dt = time - prev_time;
        prev_time = time;

        dt = glm::min(dt, 0.01f); //smaller cap for stability

        frames++;

        if (time - lastTime >= 1.0){
            std::cout << "FPS: " << frames << std::endl;
            frames = 0;
            lastTime = time;
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, upVector);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);

        

        for (int i = 0; i < n*n; i++) {
            for (int j = i + 1; j < n*n; j++) {

                glm::vec3 delta = particles[i].position - particles[j].position;

                float dist = glm::length(delta);
                
                float minDist = particles[i].radius + particles[j].radius;
                

                if (dist < minDist && dist > 0.00001f) { //the later condition is to avoid normalizing a zero length vector  
                    glm::vec3 n = glm::normalize(delta);

                    float v1n = glm::dot(velocities[i], n);
                    float v2n = glm::dot(velocities[j], n);

                    glm::vec3 v1t = velocities[i] - v1n * n;
                    glm::vec3 v2t = velocities[j] - v2n * n;

                    // Exchange the normal components
                    velocities[i] = v1t + v2n * n;
                    velocities[j] = v2t + v1n * n;

                    float penetration = minDist - dist; 
                    

                    particles[i].position += n * (penetration*0.5f);//in case of overlap, seperate them completely 
                    particles[j].position -= n * (penetration*0.5f);
                }
            }
        }
        




        for (int i = 0;i<n*n;i++){
            particles[i].position +=velocities[i]*dt; //update their positions. verlet integration is advised. 
                                                        //but for now, euler integration works fine
            
        }

        

        for (int i = 0;i<n*n;i++){
            glm::mat4 trans = glm::translate(glm::mat4(1.0f), particles[i].position); 
            particles[i].draw(particle_vaos[i], trans, view, projection); //draw the particle
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    
    glfwTerminate();


    return 0;
}


void processinput(GLFWwindow* window, glm::vec3 &cameraPos, glm::vec3 &cameraTarget){
    float cam_speed = 0.002;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
        cameraPos.y+=cam_speed;
        cameraTarget.y+=cam_speed;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
        cameraPos.y-=cam_speed;
        cameraTarget.y-=cam_speed;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
        cameraPos.x-=cam_speed;
        cameraTarget.x-=cam_speed;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
        cameraPos.x+=cam_speed;
        cameraTarget.x+=cam_speed;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        cameraPos.z+=cam_speed;
        cameraTarget.z+=cam_speed;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        cameraPos.z-=cam_speed;
        cameraTarget.z-=cam_speed;
    }
}
