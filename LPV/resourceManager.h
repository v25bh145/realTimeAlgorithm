#pragma once
// C++
#include <iostream>
#include <unordered_map>
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

class ResourceManager {
private:
	// 外部导入模型
	std::unordered_map<std::string, std::pair<Model*, glm::mat4>> modelHashMap;
	// 内建模型
	std::unordered_map<std::string, std::pair<unsigned, glm::mat4>> VAOHashMap;
	std::unordered_map<std::string, unsigned> textureHashMap;
	// 全局变量
	std::unordered_map<std::string, glm::vec3> globalVec3Map;
	Camera* camera;
	//  point light only
	glm::vec3 lightPos;
	glm::vec3 lightDiffuse;
	float nearPlane;
	float farPlane;
	ResourceManager(glm::vec3 cameraPos = { 0.f, 0.f, 3.f }, glm::vec3 cameraUp = { 0.f, 1.f, 0.f }, float cameraYaw = YAW, float cameraPitch = PITCH) {
		camera = new Camera(cameraPos, cameraUp, cameraYaw, cameraPitch);
		lightPos = { 0.f, 0.f, 0.f };
		lightDiffuse = { 1.f, 1.f, 1.f };
		nearPlane = 0.f;
		farPlane = 100.f;
	}
public:
	static ResourceManager* get(glm::vec3 cameraPos = {0.f, 0.f, 3.f}, glm::vec3 cameraUp = {0.f, 1.f, 0.f}, float cameraYaw = YAW, float cameraPitch = PITCH) {
		static bool initFlag = false;
		static ResourceManager* resourceManager = nullptr;

		if (initFlag) return resourceManager;
		initFlag = true;
		resourceManager = new ResourceManager(cameraPos, cameraUp, cameraYaw, cameraPitch);
		return resourceManager;
	}
	Camera* getCamera();
	void setPointLightInformation(glm::vec3 lightPos, glm::vec3 lightDiffuse, float nearPlane, float farPlane);
	glm::vec3 getPointLightPos();
	glm::vec3 getPointLightDiffuse();
	float getNearPlane();
	float getFarPlane();


	void setModel(std::string modelName, Model* model, glm::mat4 modelTransform);
	std::pair<Model*, glm::mat4> deleteModel(std::string modelName);
	std::pair<Model*, glm::mat4> getModel(std::string modelName);

	void setTexture(std::string textureName, unsigned texture);
	unsigned deleteTexture(std::string textureName);
	unsigned getTexture(std::string textureName);

	void setVAO(std::string VAOName, unsigned VAO, glm::mat4 VAOTransform);
	std::pair<unsigned, glm::mat4> deleteVAO(std::string VAOName);
	std::pair<unsigned, glm::mat4> getVAO(std::string VAOName);

	void setGlobalVec3(std::string varName, glm::vec3 var);
	glm::vec3 deleteGlobalVec3(std::string varName);
	glm::vec3 getGlobalVec3(std::string varName);
};