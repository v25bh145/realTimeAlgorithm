#include "LPVPass.h"

// helper:

// 1. 输出纹理值(float)

//float* data = new float[512 * 512 * 3];
//glBindTexture(GL_TEXTURE_CUBE_MAP, this->fluxMap);
//glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 2, 0, GL_RGB, GL_FLOAT, data);
//cout << data[2] << endl;

// 2. 输入六面体天空盒

//stbi_set_flip_vertically_on_load(false);
//vector<string> paths = {
//    string("../assets/skybox/skybox0.png"),
//    string("../assets/skybox/skybox1.png"),
//    string("../assets/skybox/skybox2.png"),
//    string("../assets/skybox/skybox3.png"),
//    string("../assets/skybox/skybox4.png"),
//    string("../assets/skybox/skybox5.png"),
//};
//vector<unsigned char*> datas;
//int textureSize = 0;
//int nrComponents;
//unsigned textureExample = loadCubemap(paths, textureSize, nrComponents, datas);

// 3. Render()方法没有完全封装

// bind FBO
// use shader
// clear buffer [!important]
// set viewport
// while
//      bind VAO
//      draw
// clear VAO
// clear FBO [!important]

void GetShadowSamplePass::initGlobalSettings()
{
    // light information
    ResourceManager::get()->setPointLightInformation({ 10.f, 10.f, 10.f }, { 1.f, 1.0f, 1.f }, 1.f, 25.f);
}

void GetShadowSamplePass::initShader()
{
	this->shader = new Shader("getShadowSample.vert", "getShadowSample.frag", "getShadowSample.geom");
    this->shader->use();
}
void GetShadowSamplePass::initTexture()
{
	glGenFramebuffers(1, &this->FBO);
    //this->FBO = 0;
	glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
    unsigned shadowDepthMap, worldPosMap, fluxMap;
	// texture
    createTextureCubeMapNull(shadowDepthMap, GetShadowSamplePass::SHADOW_WIDTH, GetShadowSamplePass::SHADOW_HEIGHT, GL_DEPTH_COMPONENT);
	createTextureCubeMapNull(worldPosMap, GetShadowSamplePass::SHADOW_WIDTH, GetShadowSamplePass::SHADOW_HEIGHT, GL_RGB);
	createTextureCubeMapNull(fluxMap, GetShadowSamplePass::SHADOW_WIDTH, GetShadowSamplePass::SHADOW_HEIGHT, GL_RGB);

    ResourceManager::get()->setTexture("worldPosMap", worldPosMap);
    ResourceManager::get()->setTexture("fluxMap", fluxMap);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowDepthMap, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, worldPosMap, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, fluxMap, 0);
    GLenum bufs[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, bufs);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!: status=" << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GetShadowSamplePass::initScene()
{
    /* light info
    *  geom & transform
    */
    // light transform
    ResourceManager* pResourceManager = ResourceManager::get();
    float nearPlane = pResourceManager->getNearPlane();
    float farPlane = pResourceManager->getFarPlane();
    vec3 pointLightPos = pResourceManager->getPointLightPos();
    vec3 pointLightDiffuse = pResourceManager->getPointLightDiffuse();

    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, nearPlane, farPlane);
    std::vector<glm::mat4> shadowTransforms;
    shadowTransforms.push_back(shadowProj * glm::lookAt(pointLightPos, pointLightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(pointLightPos, pointLightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(pointLightPos, pointLightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(pointLightPos, pointLightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(pointLightPos, pointLightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(pointLightPos, pointLightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    // shader set
    this->shader->setFloat("farPlane", farPlane);
    this->shader->setVec3("light.position", pointLightPos);
    this->shader->setVec3("light.diffuse", pointLightDiffuse);
    for (unsigned i = 0; i < 6; ++i) {
        this->shader->setMat4("lightShadowMatrices[" + to_string(i) + "]", shadowTransforms[i]);
    }
	/* scene info
    * geom & transform & shader set
    */
    // scene geom
    Model* nanosuit = new Model("../models/nanosuit/nanosuit.obj");
    pResourceManager->setModel("nanosuit", nanosuit, mat4(1.f));
}

void GetShadowSamplePass::Render()
{
    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
    this->shader->use();

    // !important
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, GetShadowSamplePass::SHADOW_WIDTH, GetShadowSamplePass::SHADOW_HEIGHT);

    pair<Model*, mat4> modelPair = ResourceManager::get()->getModel("nanosuit");
    this->shader->setMat4("model", modelPair.second);
    modelPair.first->Draw(*this->shader);

    // !important
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OutputCubeMapPass::initGlobalSettings()
{
    // do nothing
}

void OutputCubeMapPass::initShader()
{
    this->shader = new Shader("outputCubeMap.vert", "outputCubeMap.frag");
    this->shader->use();
}

void OutputCubeMapPass::initTexture()
{
    this->shader->setInt("skyboxTexture", 0);
}

void OutputCubeMapPass::initScene()
{
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
    
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
    
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
    
        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,
    
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };
    // screen VAO
    unsigned skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    // NDC
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    ResourceManager::get()->setVAO("skyboxVAO", skyboxVAO, mat4(1.f));
}

void OutputCubeMapPass::Render()
{
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glDepthFunc(GL_LEQUAL);
    this->shader->use();

    mat4 view = mat4(mat3(ResourceManager::get()->getCamera()->GetViewMatrix())); // remove translation from the view matrix
    mat4 projection = perspective(radians(ResourceManager::get()->getCamera()->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    this->shader->setMat4("view", view);
    this->shader->setMat4("projection", projection);

    glDisable(GL_DEPTH_TEST);
    pair<unsigned, mat4> VAOPair = ResourceManager::get()->getVAO("skyboxVAO");
    glBindVertexArray(VAOPair.first);
    glActiveTexture(GL_TEXTURE0);
    unsigned skyboxTexture = ResourceManager::get()->getTexture(this->textureName);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    //glDepthFunc(GL_LESS);
}


void Output2DPass::initGlobalSettings()
{
    // do nothing
}

void Output2DPass::initShader()
{
    this->shader = new Shader("output2D.vert", "output2D.frag");
    this->shader->use();
}

void Output2DPass::initTexture()
{
    this->shader->setInt("screenTexture", 0);
}

void Output2DPass::initScene()
{
    float screenVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
    // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
    };
    // screen VAO
    unsigned screenVAO, screenVBO;
    glGenVertexArrays(1, &screenVAO);
    glGenBuffers(1, &screenVBO);
    glBindVertexArray(screenVAO);
    glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
    // NDC
    glBufferData(GL_ARRAY_BUFFER, sizeof(screenVertices), &screenVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    ResourceManager::get()->setVAO("screenVAO", screenVAO, mat4(1.f));
}

void Output2DPass::Render()
{
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    this->shader->use();
    pair<unsigned, mat4> VAOPair = ResourceManager::get()->getVAO("screenVAO");
    glBindVertexArray(VAOPair.first);
    //glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    unsigned screenTexture = ResourceManager::get()->getTexture(this->textureName);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}