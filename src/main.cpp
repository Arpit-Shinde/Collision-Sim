// g++ -std=c++23 src/main.cpp src/glad.c -o src/main.exe -Iinclude -Llib -lglfw3 -lopengl32 -lgdi32 ; src/main.exe (ignore this command)
/*

Version 3.0

Description : 
    -Basic implementation of particle collision simulation

Improvements made:
    -Implemented simple spatial grid
    -Transitioned from O(N^2) to approximately O(N) for uniform distribution
    -Pre-allocated "capacity" tracked flat arrays for spatial grids
    
     
    
Changes to be made:
    -write struct to store particle reference (x,y,index)
    -write compute shader
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
float radius = 0.02f;
float restitution = 1.0f;
int g = (int)1/radius; //number of cells = grid_size*grid_size
std::vector <float> color = {1.0,0.0,1.0};

void find_neighbour_cells(std::vector<int> &neighbours, int id, int g){
    neighbours[0] = id;
    neighbours[1] = id+1;
    neighbours[2] = id-1;
    neighbours[3] = id+g;
    neighbours[4] = id-g;
    neighbours[5] = id+1+g;
    neighbours[6] = id+1-g;
    neighbours[7] = id-1+g;
    neighbours[8] = id-1-g;

}

void resolve_collision(std::vector <glm::vec2> &positions, std::vector<glm::vec2> &velocities, int i, int j){
    glm::vec2 delta = positions[i] - positions[j];

    float dist = (positions[i].x - positions[j].x)*(positions[i].x - positions[j].x) + (positions[i].y - positions[j].y)*(positions[i].y - positions[j].y);
    
    float minDist = 4*radius*radius; //assuming simulation for identical bodies

    // std::cout<<"resolving\n";
    

    if (dist < minDist && dist > 0.00001f) { //the later condition is to avoid normalizing a zero length vector 
        
        glm::vec2 n = glm::normalize(delta);

        float v1n = glm::dot(velocities[i], n);
        float v2n = glm::dot(velocities[j], n);

        glm::vec2 v1t = velocities[i] - v1n * n;
        glm::vec2 v2t = velocities[j] - v2n * n;

        // std::cout<<"putting velocities"<<"\n";

        // Exchange the normal components
        velocities[i] = v1t + (v1n + (1 + restitution) * (v2n - v1n) * 0.5f) * n;
        velocities[j] = v2t + (v2n + (1 + restitution) * (v1n - v2n) * 0.5f) * n;

        // std::cout<<"finding penetration"<<"\n";

        float penetration = sqrt(minDist) - sqrt(dist); 
        
        // std::cout<<"applying penetration"<<"\n";
        positions[i] += n * (penetration*0.5f);//in case of overlap, seperate them completely 
        positions[j] -= n * (penetration*0.5f);
        // std::cout<<"applied penetration"<<"\n";
    }
}

std::vector<float> get_base_vertices(){
    std::vector<float> vertices;
    
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(color[0]); //color of the particles (center)
    vertices.push_back(color[1]);
    vertices.push_back(color[2]);

    for (int i = 0; i <= resolution; i++) {
        float angle = i * 2.0f * std::numbers::pi_v<float> / resolution;

        vertices.push_back(radius* std::cos(angle));
        vertices.push_back(radius* std::sin(angle));
        vertices.push_back(0.0f);
        vertices.push_back(color[0]); //color of the particles 
        vertices.push_back(color[1]);
        vertices.push_back(color[2]);
    }

    return vertices;
}

std::vector <glm::vec2> generate_random_velocities(float min, float max){
    std::vector <glm::vec2> velocities;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution <float> random_vel(min,max); //set the initial random velocities here

    for (int i = 0;i<n*n;i++){
        velocities.push_back(glm::vec2(random_vel(gen), random_vel(gen)));
    }

    return velocities;
}

std::vector <glm::vec2> generate_initial_positions(){
    std::vector <glm::vec2> positions(n*n);
    int index=0;
    for (int y = 1; y <= n; y += 1) {
        for (int x = 1; x <= n; x += 1) {
            float x_pos = (-1 - 1/(float)n) + (2*x)/(float)n;
            float y_pos = (-1 - 1/(float)n) + (2*y)/(float)n;
            positions[index++] = glm::vec3(x_pos, y_pos,0);
        }
    }
    return positions;
}

void processinput(GLFWwindow* window){
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }
}

int main() {
    GLFWwindow* window = init_window();

    unsigned int shaderProgram = linkShaders();

    std::vector <float> vertices = get_base_vertices();

    std::vector <glm::vec2> positions = generate_initial_positions();
    std::vector <glm::vec2> velocities = generate_random_velocities(-0.05f,0.05f);
    
    unsigned int instanceVBO;
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(glm::vec2)*n*n, positions.data(), GL_STATIC_DRAW); 
    glBindBuffer(GL_ARRAY_BUFFER,0);

    std::cout<<g;

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
    
    float prev_time = 0;
    double lastTime = glfwGetTime();
    int frames = 0;

    //init a grid to store particle positions
    std::vector <std::vector<std::array<float,3>>> grid(g*g); // x,y,index. index will be a float. later convert it to int when needed
    std::vector <std::vector<std::array<float,3>>> vel_grid(g*g);
    std::vector<int> capacities(g*g,0);

    for (int i = 0;i<g*g;i++){  //fill in the grid once. in render loop we will overwrite the values instead of reallocation.
        for (int j = 0;j<(n*n)/(g*g)+100;j++){ 
            grid[i].push_back({0,0,0});
            vel_grid[i].push_back({0,0,0});
        }
    }

    std::vector <int> neighbours(9,0);//init a neighbour grid where we will store id of neighbours in render loop

    int count=0; //to track average fps
    int fps = 0;

    while (!glfwWindowShouldClose(window)){

        if (count==60){ //stop animation after 60
            break;
        }

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

        //assign particles & their velocities to their cells in grid and vel_grid respectively. check wall collision here only
        //can optimise wall collisions by creating a flag called changed = false and update it in if blocks. see it later
        for (int i=0;i<n*n;i++){

            float x = positions[i].x;
            float y = positions[i].y;
            // Lower wall (y = -1)
            if (y - radius < -1.0f) {
                positions[i].y = -1.0f + radius;  // Snap to surface
                velocities[i].y = -velocities[i].y * restitution;
            }

            // Upper wall (y = 1)
            if (y + radius > 1.0f) {
                positions[i].y = 1.0f - radius;   // Snap to surface
                velocities[i].y = -velocities[i].y * restitution;
            }

            // Left wall (x = -1)
            if (x - radius < -1.0f) {
                positions[i].x = -1.0f + radius;  // Snap to surface
                velocities[i].x = -velocities[i].x * restitution;
            }

            // Right wall (x = 1)
            if (x + radius > 1.0f) {
                positions[i].x = 1.0f - radius;   // Snap to surface
                velocities[i].x = -velocities[i].x * restitution;
            }

            float updated_x = (positions[i].x * (g/2));
            float updated_y = (positions[i].y * (g/2));

            if (updated_x >= (g/2)) updated_x = (g/2)-1;
            if (updated_y >= (g/2)) updated_y = (g/2)-1;

            int id = floor(updated_x) + g*floor(updated_y) + (int)( (g/2)*(g+1) );
            
            grid[id][capacities[id]] = {updated_x,updated_y,(float)i};
            vel_grid[id][capacities[id]] = {velocities[i].x, velocities[i].y, (float)i};
            capacities[id]+=1;
            

        }

        //check collisions here; loop through particles instead of cells
        for (int i=0;i<n*n;i++){
            float updated_x = (positions[i].x * (g/2));
            float updated_y = (positions[i].y * (g/2));

            if (updated_x >= (g/2)) updated_x = (g/2)-1;
            if (updated_y >= (g/2)) updated_y = (g/2)-1;

            int id = floor(updated_x) + g*floor(updated_y) + (int)( (g/2)*(g+1) );
            
            //find neighbours
            find_neighbour_cells(neighbours, id,g);

            for (int j = 0;j<9;j++){

                int neighbour_cell_id = neighbours[j];
                if (neighbour_cell_id<0 || neighbour_cell_id>=g*g) continue;
                int cap = capacities[neighbour_cell_id]; //capacity of neighbour cell
                
                for (int k =0;k<cap;k++){
                    int nei_index = (int) grid[neighbour_cell_id][k][2];

                    if (nei_index<=i) continue;//if the particle has been already checked

                    else{
                        //check collision of both of them here. write a function resolve_collision.
                        resolve_collision(positions, velocities, i, nei_index);
                        
                    }   
                }
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

        for (int i = 0;i<n*n;i++){
            positions[i]+=velocities[i]*dt;
        }

        for (int i=0;i<g*g;i++){
            capacities[i]=0;
        }

        glBufferSubData(
            GL_ARRAY_BUFFER,
            0,
            sizeof(glm::vec2) * n*n,
            positions.data()
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