#pragma once
#include <vector>




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


std::vector<Vertex> get_base_vertices(int resolution, std::vector<float> color, float radius);

std::vector <OffsetVertex> generate_initial_positions(int n);

std::vector <Velocity> generate_random_velocities(float max, int n);