#pragma once
#include <glad/glad.h>
#include <iostream>

const char* vertexShaderSource = R"(
    #version 460 core

    struct Vertex {
        vec4 position;
        vec4 color;
    };

    layout(std430, binding = 0) buffer vertexData {
        Vertex base_vertices[];
    };

    layout(std430, binding = 1) buffer offsetData {
        vec4 offsets[]; // Assuming offsets are vec4 (x, y, z,w)
    };

    layout(std430, binding = 2) buffer velocity_data {
        vec4 velocities[]; // Assuming offsets are vec4 (x, y, z,w)
    };

    out vec4 fColor;

    float max_velocity = sqrt(2)*0.05f;

    void main() {
        Vertex v = base_vertices[gl_VertexID];
        vec4 offset = offsets[gl_InstanceID];
        
        vec4 pos = v.position + offset;
        gl_Position = vec4(pos);

        vec2 vel = velocities[gl_InstanceID].xy;
        float velocity = length(vel); // Using length() is cleaner than manual sqrt/sum

        float t = clamp(velocity / max_velocity, 0.0, 1.0);

        vec3 yellow = vec3(1.0, 1.0, 0.0);
        vec3 teal = vec3(0.24, 0.88, 1.0);

        fColor = vec4(mix(teal,yellow, t), 1.0);
    }
)";

const char* fragmentShaderSource = R"(
    #version 460 core
    
    out vec4 FragColor;
    in vec4 fColor;
    
    void main() {
        FragColor = vec4(fColor); 
    }
)";

const char* assign_computeShaderSource = R"(
    #version 460 core

    struct ParticleReference {
        float x;
        float y;
        int index;
        int id;
    };

    layout(local_size_x=1) in;

    layout(std430, binding =1) buffer pos_data{
        vec4 positions[];
    };

    layout(std430, binding =3) buffer pos_grid_data{
        ParticleReference grid[];
    };

    layout(std430, binding =4) buffer capacity_data{
        ivec4 capacities[];
    };

    layout(std430, binding =2) buffer velocity_data{
        vec4 velocities[];
    };


    uniform vec4 essentials; 

    void main(){

        uint i = gl_GlobalInvocationID.x;

        float radius = essentials.x;
        float restitution = essentials.y;
        float dt = essentials.z;
        int n = int(essentials.w);
        float g1 = (1.0/radius);
        int g = int(g1);
        int max_particles_per_cell = int((n*n)/(g*g)) + 100;

        float x = positions[i].x; //positions[i] will give offset vertex struct
        float y = positions[i].y;
        
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

        int id = int(floor(updated_x)) + g*int(floor(updated_y)) + int( (g/2)*(g+1) );

        grid[id * max_particles_per_cell + capacities[id].x] = ParticleReference(updated_x, updated_y, int(i), id);
        
        atomicAdd(capacities[id].x, 1);
        
        
    }
    
)";


const char* resolve_computeShaderSource = R"(
    #version 460 core

    struct ParticleReference {
        float x;
        float y;
        int index;
        int id;
    };

    struct Neighbour{
        int x,y,z,w;
    };

    layout(local_size_x=1) in;

    layout(std430, binding =1) buffer pos_data{
        vec4 positions[];
    };

    layout(std430, binding =3) buffer pos_grid_data{
        ParticleReference grid[];
    };

    layout(std430, binding =4) buffer capacity_data{
        ivec4 capacities[];
    };

    layout(std430, binding =2) buffer velocity_data{
        vec4 velocities[];
    };

    

    uniform vec4 essentials; 
    float radius = essentials.x;
    float restitution = essentials.y;
    float dt = essentials.z;
    int n = int(essentials.w);
    float g1 = (1.0/radius);
    int g = int(g1);
    int max_particles_per_cell = int((n*n)/(g*g)) + 100;


    // We pass the indices of the particles involved in the collision
    void resolve_collision(int i, int j) {
        vec2 posI = positions[i].xy;
        vec2 posJ = positions[j].xy;
        vec2 velI = velocities[i].xy;
        vec2 velJ = velocities[j].xy;

        vec2 delta = posI - posJ;
        float distSq = dot(delta, delta); // Optimized: use dot product instead of manual x*x + y*y
        float minDist = 4.0 * radius * radius;

        if (distSq < minDist && distSq > 0.00001) {
            float dist = sqrt(distSq);
            vec2 n = delta / dist; // Normalize

            // --- Physics Logic ---
            float v1n = dot(velI, n);
            float v2n = dot(velJ, n);

            vec2 v1t = velI - v1n * n;
            vec2 v2t = velJ - v2n * n;

            // Exchange normal components
            velI = v1t + (v1n + (1.0 + restitution) * (v2n - v1n) * 0.5) * n;
            velJ = v2t + (v2n + (1.0 + restitution) * (v1n - v2n) * 0.5) * n;

            // Penetration resolution
            float penetration = sqrt(minDist) - dist;
            posI += n * (penetration * 0.5);
            posJ -= n * (penetration * 0.5);

            // --- Write back to Global Buffers ---
            positions[i].xy = posI;
            positions[j].xy = posJ;
            velocities[i].xy = velI;
            velocities[j].xy = velJ;
        }
    }

    void main(){

        uint i = gl_GlobalInvocationID.x;

        float updated_x = (positions[i].x * (g/2));
        float updated_y = (positions[i].y * (g/2));

        if (updated_x >= (g/2)) updated_x = (g/2)-1;
        if (updated_y >= (g/2)) updated_y = (g/2)-1;

        int id = int(floor(updated_x)) + g*int(floor(updated_y)) + int( (g/2)*(g+1) );

        int neighbours[9] = int[9](id,id+1,id-1,id+g,id-g,id+1+g,id+1-g,id-1+g,id-1-g);

        for (int j = 0;j<9;j++){
            int neighbour_cell_id = neighbours[j];

            if (neighbour_cell_id<0 || neighbour_cell_id>=g*g) continue;
            
            int cap = capacities[neighbour_cell_id].x;
            
            for (int k=0;k<cap;k++){
                int nei_index = grid[neighbour_cell_id*max_particles_per_cell + k].index;

                if (nei_index<=i) continue; //i is the index of current particle (or id of current thread)
                
                else{ 
                    resolve_collision(int(i), nei_index);    
                }
                
                
            }
        }

        
        
    }
    
)";

const char* integrate_computeShaderSource = R"(
    #version 460 core

    layout(local_size_x=1) in;

    layout(std430, binding =1) buffer pos_data{
        vec4 positions[];
    };

    layout(std430, binding =2) buffer velocity_data{
        vec4 velocities[];
    };

    uniform vec4 essentials; 

    void main(){
        uint i = gl_GlobalInvocationID.x;
        
        float dt = essentials.z;
        
        // Simple Euler integration
        positions[i].x += velocities[i].x * dt;
        positions[i].y += velocities[i].y * dt;
        
    }
)";

unsigned int compileVertexShader(){
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    int success;
    char infolog[512];

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if (!success){
        glGetShaderInfoLog(vertexShader, 512, NULL, infolog);
        std::cout<<"vertex shader compilation failed\n"<<infolog<<"\n";
    }
    return vertexShader;
}

unsigned int compile_assign_ComputeShader(){
    unsigned int computeShader;
    computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &assign_computeShaderSource, NULL);
    glCompileShader(computeShader);
    int success;
    char infolog[512];

    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);

    if (!success){
        glGetShaderInfoLog(computeShader, 512, NULL, infolog);
        std::cout<<"assign compute shader compilation failed\n"<<infolog<<"\n";
    }
    return computeShader;
}

unsigned int compile_resolve_ComputeShader(){
    unsigned int computeShader;
    computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &resolve_computeShaderSource, NULL);
    glCompileShader(computeShader);
    int success;
    char infolog[512];

    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);

    if (!success){
        glGetShaderInfoLog(computeShader, 512, NULL, infolog);
        std::cout<<"resolve compute shader compilation failed\n"<<infolog<<"\n";
    }
    return computeShader;
}

unsigned int compile_integrate_ComputeShader(){
    unsigned int computeShader;
    computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &integrate_computeShaderSource, NULL);
    glCompileShader(computeShader);
    int success;
    char infolog[512];

    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);

    if (!success){
        glGetShaderInfoLog(computeShader, 512, NULL, infolog);
        std::cout<<"integrate compute shader compilation failed\n"<<infolog<<"\n";
    }
    return computeShader;
}




unsigned int compileFragmentShader(){
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource,NULL);
    glCompileShader(fragmentShader);
    int success2;
    char infolog2[512];

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success2);

    if (!success2){
        glGetShaderInfoLog(fragmentShader, 512, NULL, infolog2);
        std::cout<<"fragment shader compilation failed\n"<<infolog2<<"\n";
    }
    return fragmentShader;
}

unsigned int linkShaders(){
    unsigned int vertexShader = compileVertexShader();
    unsigned int fragmentShader = compileFragmentShader();
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    return shaderProgram;
}

unsigned int get_assign_computeProgram(){
    unsigned int computeShader = compile_assign_ComputeShader();
    unsigned int computeProgram = glCreateProgram();
    glAttachShader(computeProgram, computeShader);
    glLinkProgram(computeProgram);

    return computeProgram;
}

unsigned int get_resolve_computeProgram(){
    unsigned int computeShader = compile_resolve_ComputeShader();
    unsigned int computeProgram = glCreateProgram();
    glAttachShader(computeProgram, computeShader);
    glLinkProgram(computeProgram);

    return computeProgram;
}

unsigned int get_integrate_computeProgram(){
    unsigned int computeShader = compile_integrate_ComputeShader();
    unsigned int computeProgram = glCreateProgram();
    glAttachShader(computeProgram, computeShader);
    glLinkProgram(computeProgram);
    return computeProgram;
}