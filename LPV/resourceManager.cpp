#include "resourceManager.h"

Camera* ResourceManager::getCamera()
{
	return this->camera;
}

void ResourceManager::setPointLightInformation(glm::vec3 lightPos, glm::vec3 lightDiffuse, float nearPlane, float farPlane) {
	this->lightPos = lightPos;
	this->lightDiffuse = lightDiffuse;
	this->nearPlane = nearPlane;
	this->farPlane = farPlane;
}
glm::vec3 ResourceManager::getPointLightPos()
{
	return this->lightPos;
}
glm::vec3 ResourceManager::getPointLightDiffuse()
{
	return this->lightDiffuse;
}
float ResourceManager::getNearPlane()
{
	return this->nearPlane;
}
float ResourceManager::getFarPlane()
{
	return this->farPlane;
}
void ResourceManager::setModel(string modelName, Model* model, glm::mat4 modelTransform) {
	this->modelHashMap.insert({ modelName, {model, modelTransform} });
}
std::pair<Model*, glm::mat4> ResourceManager::deleteModel(string modelName) {
	auto res = this->modelHashMap.find(modelName);
	if (res == this->modelHashMap.end()) {
		std::cout << "WARNING: can't find anything in deleteModel() while name = " << modelName << std::endl;
		return { nullptr, glm::mat4(1.f) };
	}
	else {
		std::pair<Model*, glm::mat4> resModel = res->second;
		this->modelHashMap.erase(modelName);
		return resModel;
	}
}
std::pair<Model*, glm::mat4> ResourceManager::getModel(string modelName) {
	auto res = this->modelHashMap.find(modelName);
	if (res == this->modelHashMap.end()) {
		std::cout << "WARNING: can't find anything in getModel() while name = " << modelName << std::endl;
		return { nullptr, glm::mat4(1.f) };
	}
	else {
		return res->second;
	}
}

void ResourceManager::setTexture(string textureName, unsigned texture) {
	this->textureHashMap.insert({ textureName, texture });
}
unsigned ResourceManager::deleteTexture(string textureName) {
	auto res = this->textureHashMap.find(textureName);
	if (res == this->textureHashMap.end()) {
		std::cout << "WARNING: can't find anything in deleteTexture() while name = " << textureName << std::endl;
		return NULL;
	}
	else {
		unsigned resModel = res->second;
		this->textureHashMap.erase(textureName);
		return resModel;
	}
}
unsigned ResourceManager::getTexture(string textureName) {
	auto res = this->textureHashMap.find(textureName);
	if (res == this->textureHashMap.end()) {
		std::cout << "WARNING: can't find anything in getTexture() while name = " << textureName << std::endl;
		return NULL;
	}
	else {
		return res->second;
	}
}

void ResourceManager::setVAO(string VAOName, unsigned VAO, glm::mat4 VAOTransform)
{
	this->VAOHashMap.insert({ VAOName, {VAO, VAOTransform} });
}

std::pair<unsigned, glm::mat4> ResourceManager::deleteVAO(string VAOName)
{
	auto res = this->VAOHashMap.find(VAOName);
	if (res == this->VAOHashMap.end()) {
		std::cout << "WARNING: can't find anything in deleteVAO() while name = " << VAOName << std::endl;
		return { NULL, glm::mat4(1.f) };
	}
	else {
		std::pair<unsigned, glm::mat4> resModel = res->second;
		this->VAOHashMap.erase(VAOName);
		return resModel;
	}
}

std::pair<unsigned, glm::mat4> ResourceManager::getVAO(string VAOName)
{
	auto res = this->VAOHashMap.find(VAOName);
	if (res == this->VAOHashMap.end()) {
		std::cout << "WARNING: can't find anything in getVAO() while name = " << VAOName << std::endl;
		return { NULL, glm::mat4(1.f) };
	}
	else {
		return res->second;
	}
}
