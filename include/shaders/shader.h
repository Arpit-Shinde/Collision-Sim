//standard shaders with no instancing

#pragma once
#include <glad/glad.h>
#include <iostream>

const char* vertexShaderSource = R"(
    #version 330 core

    layout (location = 0) in vec3 vertex_coords;
    

    
    
    uniform mat4 trans;
    uniform mat4 view;
    uniform mat4 projection;

    void main() {
        
        gl_Position = projection * view * trans * vec4(vertex_coords,1.0);
        
        
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    
    
    out vec4 FragColor;
    uniform vec4 in_color;
    void main() {
        FragColor = vec4(in_color); 
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