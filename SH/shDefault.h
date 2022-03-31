#ifndef SHDEFAULT_H
#define SHDEFAULT_H
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../include/shader.h"
#include "../include/camera.h"
#include "../include/model.h"
#include "../include/stb_image.h"

using namespace std;
using namespace glm;
#define SCR_WIDTH 800
#define SCR_HEIGHT 600

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void updateDeltaTime(float currentFrame);
void processInput(GLFWwindow* window);
unsigned int loadTexture(char const* path);
unsigned int loadCubemap(vector<string> paths, int& imageSize, int& nrComponents, vector<unsigned char*>& datas, bool getData = true);
void renderCube();
GLFWwindow* initWindow();
#endif