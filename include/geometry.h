#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <numbers>
#include <algorithm>
#include <random>

struct Vertex{
    float x,y,z,w;
    float r,g,b,a;
};

struct OffsetVertex{ //to store particles center
    float x,y,z,w;
};

struct Velocity{ //to store particles velocity
    float x,y,z,w;
};

struct ParticleReference{ 
    float x;
    float y;
    int index;
    int id; //just to make this struct 16-bytes aligned
};

struct Capacity{
    int x,y,z,w;
    
};

struct Neighbour{
    int x,y,z,w;
};

std::vector<Vertex> get_base_vertices(int resolution, std::vector<float> color, float radius){
    std::vector<Vertex> vertices;
    
    vertices.push_back({0.0f,0.0f,0.0f,1.0f,color[0],color[1],color[2],1.0f});

    for (int i = 0; i <= resolution; i++) {
        float angle = i * 2.0f * std::numbers::pi_v<float> / resolution;

        vertices.push_back({
            radius* std::cos(angle),
            radius* std::sin(angle),
            0.0f,
            1.0f,
            color[0],
            color[1],
            color[2],
            1.0f});

    }

    return vertices;
}

std::vector <OffsetVertex> generate_initial_positions(int n){
    std::vector <OffsetVertex> positions;
    for (int y = 1; y <= n; y += 1) {
        for (int x = 1; x <= n; x += 1) {
            float x_pos = (-1 - 1/(float)n) + (2*x)/(float)n;
            float y_pos = (-1 - 1/(float)n) + (2*y)/(float)n;
            positions.push_back({x_pos, y_pos, 0.0f,0.0f});
        }
    }
    return positions;
}

std::vector <Velocity> generate_random_velocities(float min, float max, int n){
    std::vector <Velocity> velocities;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution <float> random_vel(min,max); //set the initial random velocities here

    for (int i = 0;i<n*n;i++){
        velocities.push_back({random_vel(gen), random_vel(gen),0.0f,1.0f});
    }

    return velocities;
}