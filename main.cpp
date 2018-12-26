#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaders/shader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

// Variable to hold the program
GLuint program;

// Variable to hold the buffer and array objects for the vertices
GLuint pVBO;
GLuint pVAO;
GLuint pEBO;

float planeVertices[] = {
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

// Create an array of booleans to store key presses
bool keys[] = {
    false,
    false,
    false,
    false
};

// Set the camera speed
float camSpeed = 0.0001;

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

    // Check for arrow key press
    if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
        keys[0] = true;
    }
    if (key == GLFW_KEY_UP && action == GLFW_RELEASE) {
        keys[0] = false;
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        keys[1] = true;
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE) {
        keys[1] = false;
    }
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        keys[2] = true;
    }
    if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE) {
        keys[2] = false;
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        keys[3] = true;
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE) {
        keys[3] = false;
    }

}

// Method to generate the world plane
void worldPlane() {

    // Create a VAO to hold information about the render object for the VBO data
    glGenVertexArrays(1, &pVAO);

    // Create an element buffer
    glGenBuffers(1, &pEBO);

    // Bind to the vertex array
    glBindVertexArray(pVAO);

    // Create the vertex buffer object and bind this as the active GL buffer
    glGenBuffers(1, &pVBO);
    glBindBuffer(GL_ARRAY_BUFFER, pVBO);

    // Buffer the vertex information in the vertex buffer object
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    // Create the Element Buffer object
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Setup the step data for the VBO to access x,y,z of each vertex
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Setup the step data for the VBO to access tex coords of each vertex
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    // Debind the buffers
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

// Method to generate and bind a texture
void genTexture(GLuint tex, GLenum texLoc, Shader s, const GLchar* sLocation) {

    // Load the texture data
    int width, height, nrChannels;
    unsigned char* texData = stbi_load(sLocation, &width, &height, &nrChannels, 0);

    // Check that the image data loaded successfully
    if (texData) {

        // Create the texture object
        glGenTextures(1, &tex);

        // Use the shader
        s.use();

        // Set the active texture
        glActiveTexture(texLoc);

        // Bind track tex as the currently active texture
        glBindTexture(GL_TEXTURE_2D, tex);

        // Generate the texture and mipmap
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Free the image data
        stbi_image_free(texData);

    }
    else {
        fprintf(stderr, "Failed to load texture...");
    }

}

int main(int argc, const char * argv[]) {
    
    // Check whether GLFW initialised successfully
    if(!glfwInit()) {
        fprintf(stderr, "Error loading GLFW...\n");
        return 1;
    }

    // Set the error callback handler
    glfwSetErrorCallback(error_callback);

    // Use open GL 3.3 core min
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

    // Load the shaders
    Shader s("shaders/vertex.glsl", "shaders/fragment.glsl");

    // Variables to hold the textures
    GLuint trackTex;
    GLuint carTex;

    // Generate the textures
    genTexture(trackTex, GL_TEXTURE0,  s, "assets/track.jpg");
    genTexture(trackTex, GL_TEXTURE1,  s, "assets/car.jpg");

    // Set the textures in the shader program
    s.setInt("trackTex", 0);
    s.setInt("carTex", 1);

    // Generate the world plane
    worldPlane();

    // Create the view matrix
    glm::mat4 viewMatrix;
    viewMatrix = glm::translate(viewMatrix, glm::vec3(0.0f, 0.0f, -1.0f));

    // Create the projection matrix to use a perspective camera
    glm::mat4 projectionMatrix;
    projectionMatrix = glm::perspective(glm::radians(45.0f), (float)800/600, 0.1f, 100.0f);

    // Create the mvp matrix
    glm::mat4 mvp;

    // Create the pointers to the uniform floats in the vertex shader
    GLuint mvpLoc = glGetUniformLocation(s.program, "mvp");

    // Create the position matrix of the camera
    glm::vec3 camera = glm::vec3(0.0f, 0.0f, 0.0f);

    // Create the game loop to run until the window close button or esc is clicked
    while(!glfwWindowShouldClose(window)) {

        // Check pressed keys
        if (keys[0]) {
            camera.y += 3 * camSpeed;
            camera.z -= 4 * camSpeed;
        }
        if(keys[1]) {
            camera.y -= 3 * camSpeed;
            camera.z += 4 * camSpeed;
        }
        if(keys[2]) {
            camera.x -= 5 * camSpeed;
        }
        if(keys[3]) {
            camera.x += 5 * camSpeed;
        }

        // Recalculate the model matrix
        glm::mat4 modelMatrix;
        modelMatrix = glm::translate(modelMatrix, camera);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        // Create the mvp matrix for the plane
        mvp = projectionMatrix * viewMatrix * modelMatrix;

        // Pass the data for the model, view and projection matrix to the shader
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

        // Clear to background colour
        glClearBufferfv(GL_COLOR, 0, bg);

        // Bind to the buffer to draw the plane
        glBindVertexArray(pVAO);

        // Draw the triangle
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Debind the buffer
        glBindVertexArray(0);

        // Swap the buffer to render
        glfwSwapBuffers(window);

        // Use vSync to limit the number of times the buffers are swapped
        glfwSwapInterval(1);

        // Poll key press events
        glfwPollEvents();

    }

    // Destroy vertex objects
    glDeleteVertexArrays(1, &pVAO);
    glDeleteBuffers(1, &pVBO);
    glDeleteBuffers(1, &pEBO);

    // Destroy the program
    glDeleteProgram(program);

    // Terminate GLFW window
    glfwDestroyWindow(window);

    // Terminate GLFW
    glfwTerminate();

    fprintf(stdout, "Program terminated successfully...\n");
    return 0;

}