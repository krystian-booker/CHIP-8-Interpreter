#ifndef CHIP8_INTERPRETER_RENDERER_H
#define CHIP8_INTERPRETER_RENDERER_H

// System Headers
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Renderer {
public:
    int Initialize(int _width, int _height);

    //TODO: refactor
    int WindowShouldClose();

    void BackgroundColor();

    void Draw();

    void Update();

    void Terminate();
private:
    int width = 0;
    int height = 0;
    int viewScaling = 8;

    int status = 0;
    char infoLog[512];

    unsigned int vertexShader = 0;
    unsigned int fragmentShader = 0;

    GLFWwindow* window;

    void createWindow(int _width, int _height);
    void compileVertexShader();
    void compileFragmentShader();
    void linkShaders();
};


#endif //CHIP8_INTERPRETER_RENDERER_H
