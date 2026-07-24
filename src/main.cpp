// g++ -std=c++23 src/main.cpp src/glad.c -o src/main.exe -Iinclude -Llib -lglfw3 -lopengl32 -lgdi32 ; src/main.exe (ignore this command)
/*

Version 4.0

Description : 
    -gpu accelerated implementation of 2D particle collision simulation

Improvements made:
    -changed nested vector of grid to flat vector. make sure to change access by index in render or compute loop
    -planning to remove dependency on vel_grid
    -added compute shaders to shader.h
    -passing radius to compute program through uniforms
    -removed shared neighbour array as all threads try to simultaneously write it. instead seperate neighbour array for each thread
    
    
improvements that can be made:
    -write a compute shader which clears capacity array instead of sub buffer data 
    -color the particles based on their velocity. for this, access the velocity ssbo inside vertex shader and modify fColor in it. (done)
    
key takeaways : 1. assigning struct data to variables in glsl is not as same as we do in normal cpp 
                2. dont forget to clear the capacity array
                3. we are currently clearing the capacity array through cpu. later, write a 3rd compute shader to clear the capacity array 
                4. right now, code works fine but the walls seem to shrink. maybe check wall collsion in assign compute shader rather than in resolve? (tried but failed)
                    or else write a 3rd compute shader for euler integration. (tried but failed)
                    maybe resolve_collision() is problem? commented it out and ran the code. still error, implies resolve_collsion() is not a problem
                    tried not using integrate compute shader, so that the particles dont move. the particles arent shrinking.
                    means this integrate compute shader is likely the culprit
                    yes it is. we need to update x and y coordinates of the particles. but in the integrate shader, the whole positions[i] is being updated 
                    means x,y,z,w all are being updated. w is used for perspective divide. it should not change for us. 
                    issue fixed

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
#include <geometry.h>

int resolution = 150; //resolution for the circle. more res, more round.
int n = 5; //number of particles = n*n.
float radius = 0.02; //recommended radius = 1/n for compact square packing
float restitution = 1.0f;
int g = int(1/radius); //number of cells = g*g
std::vector <float> color = {1.0,0.0,1.0};
std::vector <float> max_velocities_range = {-0.05f,0.05f};
int max_particles_per_cell = (n*n)/(g*g) + 100;




int main() {
    GLFWwindow* window = init_window();

    unsigned int shaderProgram = linkShaders();

    unsigned int assign_compute_program = get_assign_computeProgram(); //program to assign the particles to their cells
    unsigned int resolve_compute_program = get_resolve_computeProgram(); //program to resolve the collisions
    unsigned int integrate_compute_program = get_integrate_computeProgram(); //program to update positions

    std::vector <Vertex> vertices = get_base_vertices(resolution, color, radius);

    std::vector <OffsetVertex> positions = generate_initial_positions(n);
    std::vector <Velocity> velocities = generate_random_velocities(max_velocities_range[0],max_velocities_range[1],n);

    std::vector <ParticleReference> grid(g*g*max_particles_per_cell);
    std::vector<Capacity> capacities(g*g,{0,0,0,0});
    

    for (int i = 0;i<g*g*max_particles_per_cell;i++){  
        grid[i]={0,0,0,0};
    }

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int ssbovertices; //vertices data ssbo binded to 0
    glGenBuffers(1, &ssbovertices);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER,ssbovertices);
    glBufferData(GL_SHADER_STORAGE_BUFFER,vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,0,ssbovertices);

    unsigned int ssbopos; //positions data ssbo binded to 1
    glGenBuffers(1, &ssbopos);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER,ssbopos);
    glBufferData(GL_SHADER_STORAGE_BUFFER,sizeof(OffsetVertex)*positions.size(), positions.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,1,ssbopos); 
    
    unsigned int ssbovel; //velocities data ssbo binded to 2
    glGenBuffers(1, &ssbovel);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER,ssbovel);
    glBufferData(GL_SHADER_STORAGE_BUFFER,sizeof(Velocity)*velocities.size(), velocities.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,2,ssbovel);

    unsigned int ssboposgrid; //pos grid data ssbo binded to 3
    glGenBuffers(1, &ssboposgrid);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER,ssboposgrid);
    glBufferData(GL_SHADER_STORAGE_BUFFER,sizeof(ParticleReference)*grid.size(), grid.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,3,ssboposgrid);

    unsigned int ssbocap; //capacities data ssbo binded to 4
    glGenBuffers(1, &ssbocap);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER,ssbocap);
    glBufferData(GL_SHADER_STORAGE_BUFFER,sizeof(Capacity)*capacities.size(), capacities.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,4,ssbocap);

    glBindVertexArray(0);
    
    float prev_time = 0;
    double lastTime = glfwGetTime();
    int frames = 0;
    int count=0; //to track average fps
    int fps = 0;

    const GLubyte* version = glGetString(GL_VERSION);
    std::cout<<version;

    while (!glfwWindowShouldClose(window)){

        // if (count==60){ //stop animation after 60
        //     break;
        // }

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

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbocap);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, capacities.size() * sizeof(Capacity), capacities.data());

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        glUseProgram(assign_compute_program);
        glm::vec4 essentials = glm::vec4(radius, restitution, dt,(float)n);
        int loc = glGetUniformLocation(assign_compute_program, "essentials");
        glUniform4fv(loc, 1,&essentials[0]);
        glDispatchCompute(n*n, 1,1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        glUseProgram(resolve_compute_program);
        glm::vec4 essentials2 = glm::vec4(radius, restitution, dt,(float)n);
        int loc2 = glGetUniformLocation(resolve_compute_program, "essentials");
        glUniform4fv(loc2, 1,&essentials2[0]);
        glDispatchCompute(n*n, 1,1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbopos);

        glUseProgram(integrate_compute_program);
        glm::vec4 essentials3 = glm::vec4(radius, restitution, dt,(float)n);
        int loc3 = glGetUniformLocation(integrate_compute_program, "essentials");
        glUniform4fv(loc3, 1,&essentials3[0]);
        glDispatchCompute(n*n, 1,1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, vertices.size(), n*n);

        

        glfwSwapBuffers(window);
        glfwPollEvents();

        
    }

    float avg = fps/(float) count;
    std::cout<<"\n---------\navg fps = "<<avg<<"\n"; 
    
    glfwTerminate();


    return 0;
}