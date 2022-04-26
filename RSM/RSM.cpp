#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../include/stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../include/shader.h"
#include "../include/camera.h"
#include "../include/model.h"

#include <iostream>
#include <cstdlib>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

// camera
Camera camera(glm::vec3(4.0f, 4.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -135.f, -15.f);
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// meshes
unsigned int planeVAO;
// TODO: 把六个深度贴图输出出来
int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader GBufferShader("RSMGBuffer.vert", "RSMGBuffer.frag", "RSMGBuffer.geom");
    Shader shader("RSMLast.vert", "RSMLast.frag");


    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------

    Model roomModel("../models/Room/Room #1.obj");

    //float quadVertices[] = {
    //    0.f, 0.f, 0.f,  0.0f, -1.0f, 0.0f,
    //    0.f, 0.f, 5.f,  0.0f, -1.0f, 0.0f,
    //    5.f, 0.f, 0.f,  0.0f, -1.0f, 0.0f,
    //    5.f, 0.f, 5.f,  0.0f, -1.0f, 0.0f
    //};
    //unsigned quadIndices[] = {
    //    0, 1, 2,
    //    1, 2, 3
    //};
    //// quad VAO
    //unsigned quadVAO, quadVBO, quadEBO;
    //glGenVertexArrays(1, &quadVAO);
    //glGenBuffers(1, &quadVBO);
    //glGenBuffers(1, &quadEBO);
    //glBindVertexArray(quadVAO);
    //glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);
    //glEnableVertexAttribArray(0);
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    //glEnableVertexAttribArray(1);
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    //glBindVertexArray(0);
    //float boxVertices[] = {
    //0.f, 0.f, 0.f,  0.0f, 1.0f, 0.0f,
    //0.f, 0.f, 1.f,  0.0f, 1.0f, 0.0f,
    //1.f, 0.f, 0.f,  0.0f, 1.0f, 0.0f,
    //1.f, 0.f, 1.f,  0.0f, 1.0f, 0.0f
    //};
    //unsigned boxIndices[] = {
    //0, 1, 2,
    //1, 2, 3
    //};
    //// box VAO
    //unsigned boxVAO, boxVBO, boxEBO;
    //glGenVertexArrays(1, &boxVAO);
    //glGenBuffers(1, &boxVBO);
    //glGenBuffers(1, &boxEBO);
    //glBindVertexArray(boxVAO);
    //glBindBuffer(GL_ARRAY_BUFFER, boxVBO);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(boxVertices), boxVertices, GL_STATIC_DRAW);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boxEBO);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(boxIndices), boxIndices, GL_STATIC_DRAW);
    //glEnableVertexAttribArray(0);
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    //glEnableVertexAttribArray(1);
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    //glBindVertexArray(0);

    // configure depth map FBO
    // -----------------------
    const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
    unsigned RSMFBO;
    glGenFramebuffers(1, &RSMFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, RSMFBO);
    // depthMap
    unsigned depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthMap);
    for (unsigned i = 0; i < 6; ++i) {
        //glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // worldPos
    unsigned worldPosMap;
    glGenTextures(1, &worldPosMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, worldPosMap);
    for (unsigned i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
        //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, worldPosMap, 0);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    // normal
    unsigned normalMap;
    glGenTextures(1, &normalMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, normalMap);
    for (unsigned i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
        //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, normalMap, 0);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    // flux
    unsigned fluxMap;
    glGenTextures(1, &fluxMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, fluxMap);
    for (unsigned i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
        //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, fluxMap, 0);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    // attach depth texture as FBO's depth buffer
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, worldPosMap, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, normalMap, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, fluxMap, 0);
    GLenum bufs[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, bufs);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!: status=" << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // random
    srand(time(0));
    float sampleNum = 50;
    float* randomData = new float[2 * sampleNum];
    for (int i = 0; i < sampleNum; ++i) {
        randomData[i * 2] = rand() / double(RAND_MAX);
        randomData[i * 2 + 1] = rand() / double(RAND_MAX);
    }
    unsigned randomMap;
    glGenTextures(1, &randomMap);
    glBindTexture(GL_TEXTURE_1D, randomMap);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RG, sampleNum, 0, GL_RG, GL_FLOAT, &randomData[0]);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glBindTexture(GL_TEXTURE_1D, 0);

    // lighting info
    // -------------
    glm::vec3 lightPos(7.0f, 5.0f, 5.0f);

    // matrix info
    // -----------
    float near_plane = 1.0f;
    float far_plane = 100.f;
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
    std::vector<glm::mat4> shadowTransforms;
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    
    //vector<glm::mat4> quadModelTransforms;
    //glm::mat4 model1(1.f);
    //model1 = glm::translate(model1, { 0.f, 5.f, 0.f });
    //quadModelTransforms.push_back(model1);
    //glm::mat4 model2 = glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(1.f, 0.f, 0.f));
    //model2 = glm::translate(model2, { 0.f, 0.f, -5.f });
    //quadModelTransforms.push_back(model2);
    //glm::mat4 model3 = glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(-1.f, 0.f, 0.f));
    ////model3 = glm::translate(model3, { 0.f, -5.f, 0.f });
    //quadModelTransforms.push_back(model3);
    //glm::mat4 model4 = glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
    //model4 = glm::translate(model4, { 0.f, 5.f, -5.f });
    //quadModelTransforms.push_back(model4);
    //glm::mat4 model5(1.f);
    //quadModelTransforms.push_back(glm::rotate(model5, glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f)));
    //vector<glm::mat4> boxModelTransforms;
    //glm::vec3 boxTranslate = { 2.f, 1.f, 1.f };
    //glm::mat4 boxModel1 = glm::translate(glm::mat4(1.f), boxTranslate);
    //boxModelTransforms.push_back(boxModel1);
    //glm::mat4 boxModel2 = glm::translate(glm::mat4(1.f), boxTranslate);
    //boxModel2 = glm::translate(boxModel2, { 0.f, 0.f, 1.f });
    //boxModel2 = glm::rotate(boxModel2, glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
    //boxModelTransforms.push_back(boxModel2);
    //glm::mat4 boxModel3 = glm::translate(glm::mat4(1.f), boxTranslate);
    //boxModel3 = glm::translate(boxModel3, { 0.f, -1.f, 0.f });
    //boxModel3 = glm::rotate(boxModel3, glm::radians(270.f), glm::vec3(1.f, 0.f, 0.f));
    //boxModelTransforms.push_back(boxModel3);
    //glm::mat4 boxModel4 = glm::translate(glm::mat4(1.f), boxTranslate);
    //boxModel4 = glm::translate(boxModel4, { 1.f, 0.f, 0.f });
    //boxModel4 = glm::rotate(boxModel4, glm::radians(90.f), glm::vec3(0.f, 0.f, -1.f));
    //boxModelTransforms.push_back(boxModel4);
    //glm::mat4 boxModel5 = glm::translate(glm::mat4(1.f), boxTranslate);
    //boxModel5 = glm::translate(boxModel5, { 0.f, -1.f, 0.f });
    //boxModel5 = glm::rotate(boxModel5, glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
    //boxModelTransforms.push_back(boxModel5);

    // shader configuration
    // --------------------
    shader.use();
    shader.setInt("depthMap", 0);
    shader.setInt("worldPosMap", 1);
    shader.setInt("normalMap", 2);
    shader.setInt("fluxMap", 3);
    shader.setInt("randomMap", 4);
    shader.setFloat("farPlane", far_plane);
    shader.setVec3("light.position", lightPos);
    shader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
    shader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
    shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
    shader.setFloat("light.constant", 1.0f);
    shader.setFloat("light.linear", 0.09f);
    shader.setFloat("light.quadratic", 0.032f);

    GBufferShader.use();
    GBufferShader.setFloat("farPlane", far_plane);
    GBufferShader.setVec3("light.position", lightPos);
    GBufferShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
    for (unsigned i = 0; i < 6; ++i) {
        GBufferShader.setMat4("lightShadowMatrices[" + to_string(i) + "]", shadowTransforms[i]);
    }

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        

        // 1. render depth of scene to texture (from light's perspective)
        // --------------------------------------------------------------

        // render scene from light's point of view
        GBufferShader.use();

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, RSMFBO);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        //glBindVertexArray(quadVAO);
        //for (unsigned i = 0; i < 5; ++i) {
        //    glm::mat4 model = quadModelTransforms[i];
        //    GBufferShader.setMat4("model", model);
        //    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        //}
        //glBindVertexArray(0);
        //glBindVertexArray(boxVAO);
        //for (unsigned i = 0; i < 5; ++i) {
        //    glm::mat4 model = boxModelTransforms[i];
        //    GBufferShader.setMat4("model", model);
        //    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        //}
        //glBindVertexArray(0);
        GBufferShader.setMat4("model", glm::mat4(1.0f));
        roomModel.Draw(GBufferShader);
        // TEST
        //cout << "1: " << glGetError() << endl;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        

        
        

        // reset viewport
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        

        shader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setVec3("viewPos", camera.Position);

        shader.setMat4("model", glm::mat4(1.0f));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, worldPosMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, normalMap);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_CUBE_MAP, fluxMap);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_1D, randomMap);

        //glBindVertexArray(quadVAO);
        //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
        //for (unsigned i = 0; i < 5; ++i) {
        //    glm::mat4 model = quadModelTransforms[i];
        //    shader.setMat4("model", model);
        //    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        //}
        //glBindVertexArray(0);
        //glBindVertexArray(boxVAO);
        //for (unsigned i = 0; i < 5; ++i) {
        //    glm::mat4 model = boxModelTransforms[i];
        //    shader.setMat4("model", model);
        //    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        //}
        //glBindVertexArray(0);
        roomModel.Draw(shader);
        // TEST
        //cout << "2: " << glGetError() << endl;

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    //glDeleteVertexArrays(1, &quadVAO);
    //glDeleteBuffers(1, &quadVBO);
    //glDeleteBuffers(1, &quadEBO);

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}