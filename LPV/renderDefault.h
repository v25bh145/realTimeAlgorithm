#pragma once
// C++
#include <iostream>
// OpenGl
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// constructed by myself
#include "../include/shader.h"
#include "../include/camera.h"
#include "../include/model.h"
#include "../include/stb_image.h"
#include "texture.h"
#include "resourceManager.h"

#define SCR_WIDTH 800
#define SCR_HEIGHT 600

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void updateDeltaTime(float currentFrame);
void processInput(GLFWwindow* window);
GLFWwindow* initWindow();