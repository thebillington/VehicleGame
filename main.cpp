#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

// Variable to hold the program
GLuint program;

// Variable to hold the buffer and array objects for the vertices
GLuint VBO;
GLuint VAO;
GLuint EBO;

float vertices[] = {
     0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
     0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
    -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
};
unsigned int indices[] = {
    0, 1, 3,
    1, 2, 3
};

float texCoords[] = {
    0.0f, 0.0f,
    1.0f, 1.0f,
    0.5f, 1.0f
};

// Set the background colour
static const GLfloat bg[] = {0.2f, 0.3f, 0.3f, 1.0f};

// Function to handle GLFW error callbacks
void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

// Create a key callback to handle key press events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    // Check for escape key press
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

}

// Load shader and return string
string loadFile(string fName) {

    // Create an input file stream to load shaders
    std::ifstream inFile;

    // Open the file
    inFile.open(fName, ios::in);

    // Check if the file failed to load
    if(!inFile) {
        fprintf(stderr, "%s not found...\n", fName.c_str());
        exit(1);
    }

    // Buffer the string as a string stream
    std::stringstream buffer;
    buffer << inFile.rdbuf();

    // Return the string
    return buffer.str()+"\0";

}

int main(int argc, const char * argv[]) {
    
    // Check whether GLFW initialises successfully
    if(!glfwInit()) {
        fprintf(stderr, "Error loading GLFW...\n");
        return 1;
    }

    // Set the error callback handler
    glfwSetErrorCallback(error_callback);

    // Use open GL 4.1 core min
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // Set forward compatability
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Set the core profile to use newest version
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Disable resizing
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Create a window and a GL context
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vehicle Game", NULL, NULL);

    // Check if the window failed to instantiate
    if(!window) {
        fprintf(stderr, "Failed to create GLFW window...\n");
        glfwTerminate();
        return 1;
    }

    // Set the GL context to handle the window
    glfwMakeContextCurrent(window);

    // Set the key press callback function
    glfwSetKeyCallback(window, key_callback);

    // Initialise GLEW
    glewExperimental = GL_TRUE;
    glewInit();

    // Load the vertex shader from file
    const char *vertexData = loadFile("shaders/vertex.glsl").c_str();

    // Create the shaders
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

    // Set the source and destination of object for the shader then compile
    glShaderSource(vs, 1, &vertexData, NULL);
    glCompileShader(vs);

    // Check for compile errors
    int success;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);

    // If there is an error, log
    if(!success) {
        char error[512];
        glGetShaderInfoLog(vs, 512, NULL, error);
        fprintf(stderr, "Error compiling vertex shader: \n%s\nUsing default shader...\n", error);
    }

    // Load the frag shader from file
    const char *fragData = loadFile("shaders/fragment.glsl").c_str();

    // Set the source and destination of object for the shader then compile
    glShaderSource(fs, 1, &fragData, NULL);
    glCompileShader(fs);

    // Check for compile errors
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);

    // If there is an error, log
    if(!success) {
        char error[512];
        glGetShaderInfoLog(fs, 512, NULL, error);
        fprintf(stderr, "Error compiling fragment shader: \n%s\nUsing default shader...\n", error);
    }

    // Create the program to handle the shaders
    GLuint program;
    program = glCreateProgram();

    // Attach the shaders
    glAttachShader(program, vs);
    glAttachShader(program, fs);

    // Link the program
    glLinkProgram(program);

    // Check for success of linking
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    // If there is an error, log
    if(!success) {
        char error[512];
        glGetProgramInfoLog(program, 512, NULL, error);
        fprintf(stderr, "Error linking shader program: \n%s", error);
    }
    

    // Set open GL to use the shaders
    glUseProgram(program);

    // Load the container texture data
    int width, height, nrChannels;
    unsigned char* texData = stbi_load("assets/container.jpg", &width, &height, &nrChannels, 0);

    // Check that the image data loaded successfully
    if (texData) {

        // Create the texture object
        GLuint containerTex;
        glGenTextures(1, &containerTex);

        // Bind container tex as the currently active texture
        glBindTexture(GL_TEXTURE_2D, containerTex);

        // Generate the texture and mipmap
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Free the image data
        stbi_image_free(texData);

    }
    else {
        fprintf(stderr, "Failed to load texture...");
    }

    // Create a VAO to hold information about the render object for the VBO data
    glGenVertexArrays(1, &VAO);

    // Create an element buffer
    glGenBuffers(1, &EBO);

    // Bind to the vertex array
    glBindVertexArray(VAO);

    // Create the vertex buffer object and bind this as the active GL buffer
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Buffer the vertex information in the vertex buffer object
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Create the Element Buffer object
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Setup the step data for the VBO to access x,y,z of each vertex
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Setup the step data for the VBO to access tex coords of each vertex
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    // Create a translation vector
    glm::mat4 trans;

    // Rotate and scale the vector
    trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    //trans = glm::scale(trans, glm::vec3(0.5f, 0.5f, 0.5f));

    // Set trans to link to the uniform translation vector in the vertex shader
    GLuint transLoc = glGetUniformLocation(program, "transform");
    glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(trans));

    // Create the game loop to run until the window close button or esc is clicked
    while(!glfwWindowShouldClose(window)) {

        // Clear to background colour
        glClearBufferfv(GL_COLOR, 0, bg);

        // Select the shader configuration
        glUseProgram(program);

        // Draw the triangle
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        trans = glm::rotate(trans, (GLfloat)glfwGetTime() / 500.0f, glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(trans));

        // Swap the buffer to render
        glfwSwapBuffers(window);

        // Use vSync to limit the number of times the buffers are swapped
        glfwSwapInterval(1);

        // Poll key press events
        glfwPollEvents();

    }

    // Destroy the shaders
    glDeleteShader(vs);
    glDeleteShader(fs);

    // Destroy vertex objects
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // Destroy the program
    glDeleteProgram(program);

    // Terminate GLFW window
    glfwDestroyWindow(window);

    // Terminate GLFW
    glfwTerminate();

    fprintf(stdout, "Program terminated successfully...\n");
    return 0;

}