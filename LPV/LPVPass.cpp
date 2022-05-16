#include "LPVPass.h"
#include <set>

// helper:

// 1. 输出纹理值(float)

//float* data = new float[512 * 512 * 3];
//glBindTexture(GL_TEXTURE_CUBE_MAP, this->shadowFluxMap);
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

// GL_MAX_IMAGE_UNITS must >= 6

void GetShadowSamplePass::initGlobalSettings()
{
    ResourceManager::get(vec3(4.0f, 4.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -135.f, -15.f);
    // light information
    ResourceManager::get()->setPointLightInformation({ 7.0f, 5.0f, 5.0f }, { 0.6f, 0.6f, 0.6f }, 1.f, 25.f);
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
    unsigned shadowDepthMap, shadowWorldPosMap, shadowFluxMap;
	// texture
    createTextureCubeMapNull(shadowDepthMap, SHADOW_WIDTH, SHADOW_HEIGHT, GL_DEPTH_COMPONENT);
	createTextureCubeMapNull(shadowWorldPosMap, SHADOW_WIDTH, SHADOW_HEIGHT, GL_RGB);
	createTextureCubeMapNull(shadowFluxMap, SHADOW_WIDTH, SHADOW_HEIGHT, GL_RGB);

    ResourceManager::get()->setTexture("shadowWorldPosMap", shadowWorldPosMap);
    ResourceManager::get()->setTexture("shadowFluxMap", shadowFluxMap);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowDepthMap, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, shadowWorldPosMap, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, shadowFluxMap, 0);
    GLenum bufs[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, bufs);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!: status=" << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
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
    Model* nanosuit = new Model("../models/Room/Room #1.obj");
    //Model* nanosuit = new Model("../models/nanosuit/nanosuit.obj");
    pResourceManager->setModel("renderModel", nanosuit, mat4(1.f));
    pResourceManager->setGlobalVec3("boundingVolumeMin", nanosuit->boudingVolume.first);
    pResourceManager->setGlobalVec3("boundingVolumeMax", nanosuit->boudingVolume.second);
}

void GetShadowSamplePass::Render()
{
    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
    this->shader->use();

    // !important
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

    pair<Model*, mat4> modelPair = ResourceManager::get()->getModel("renderModel");
    this->shader->setMat4("model", modelPair.second);
    modelPair.first->Draw(*this->shader);

    // !important
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// fix: 不再进行VPL采样，对整个cubemap纹理进行一次渲染，将其中的fragment shader用于注入
// fix: propogation进行大改，传输所有格子，每个格子向所有方向进行若干次传播(抛弃值为0的点)

void LightInjectionPass::initGlobalSettings()
{
}

void LightInjectionPass::initShader()
{
    this->shader = new Shader("lightInjection.vert", "lightInjection.frag", "lightInjection.geom");
    this->shader->use();
}

void LightInjectionPass::initTexture()
{
    glGenFramebuffers(1, &this->FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
    auto pResourceManager = ResourceManager::get();
    vec3 gridMaxBox = pResourceManager->getGlobalVec3("boundingVolumeMax");
    vec3 gridMinBox = pResourceManager->getGlobalVec3("boundingVolumeMin");
    cout << "maxBoundingBox = " << gridMaxBox.x << ", " << gridMaxBox.y << "," << gridMaxBox.z << endl;
    cout << "minBoundingBox = " << gridMinBox.x << ", " << gridMinBox.y << "," << gridMinBox.z << endl;

    const vec3 fGridSize = (gridMaxBox - gridMinBox) / float(this->iGridTextureSize);
    pResourceManager->setGlobalVec3("fGridSize", fGridSize);
    this->shader->setVec3("fGridSize", fGridSize);
    this->shader->setVec3("gridMinBox", gridMinBox);

    unsigned gridTextureR0, gridTextureG0, gridTextureB0, gridTextureR1, gridTextureG1, gridTextureB1;
    glGenTextures(1, &gridTextureR0);
    glBindTexture(GL_TEXTURE_3D, gridTextureR0);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, this->iGridTextureSize, this->iGridTextureSize, this->iGridTextureSize);
    //glTexImage3D(GL_TEXTURE_3D, 0, GL_R32UI, this->iGridTextureSize, this->iGridTextureSize, this->iGridTextureSize, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
    //cout << "glTexImage3D " << glGetError() << endl;
    glGenTextures(1, &gridTextureR1);
    glBindTexture(GL_TEXTURE_3D, gridTextureR1);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, this->iGridTextureSize, this->iGridTextureSize, this->iGridTextureSize);

    glGenTextures(1, &gridTextureG0);
    glBindTexture(GL_TEXTURE_3D, gridTextureG0);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, this->iGridTextureSize, this->iGridTextureSize, this->iGridTextureSize);
    glGenTextures(1, &gridTextureG1);
    glBindTexture(GL_TEXTURE_3D, gridTextureG1);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, this->iGridTextureSize, this->iGridTextureSize, this->iGridTextureSize);

    glGenTextures(1, &gridTextureB0);
    glBindTexture(GL_TEXTURE_3D, gridTextureB0);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, this->iGridTextureSize, this->iGridTextureSize, this->iGridTextureSize);
    glGenTextures(1, &gridTextureB1);
    glBindTexture(GL_TEXTURE_3D, gridTextureB1);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, this->iGridTextureSize, this->iGridTextureSize, this->iGridTextureSize);

    glBindTexture(GL_TEXTURE_3D, 0);

    pResourceManager->setTexture("gridTextureR0", gridTextureR0);
    pResourceManager->setTexture("gridTextureR1", gridTextureR1);
    pResourceManager->setTexture("gridTextureG0", gridTextureG0);
    pResourceManager->setTexture("gridTextureG1", gridTextureG1);
    pResourceManager->setTexture("gridTextureB0", gridTextureB0);
    pResourceManager->setTexture("gridTextureB1", gridTextureB1);

    this->shader->setInt("gridTextureR0", 0);
    this->shader->setInt("gridTextureR1", 1);
    this->shader->setInt("gridTextureG0", 2);
    this->shader->setInt("gridTextureG1", 3);
    this->shader->setInt("gridTextureB0", 4);
    this->shader->setInt("gridTextureB1", 5);

    // test texture
    //GLuint samplesIdxInGridTexture;
    //glGenTextures(1, &samplesIdxInGridTexture);
    //glBindTexture(GL_TEXTURE_1D, samplesIdxInGridTexture);
    //glTexStorage1D(GL_TEXTURE_1D, 1, GL_RGBA32UI, 25);
    //glBindTexture(GL_TEXTURE_1D, 0);
    //this->shader->setInt("samplesIdxInGridTexture", 6);
    //pResourceManager->setTexture("samplesIdxInGridTexture", samplesIdxInGridTexture);

    unsigned depthMap;
    createTextureCubeMapNull(depthMap, INJECTION_WIDTH, INJECTION_HEIGHT, GL_DEPTH_COMPONENT);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!: status=" << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void LightInjectionPass::initScene()
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
    glBindVertexArray(0);

    ResourceManager::get()->setVAO("injectionCubemapVAO", skyboxVAO, mat4(1.f));

    vec3 pointLightPos = ResourceManager::get()->getPointLightPos();
    float nearPlane = ResourceManager::get()->getNearPlane();
    float farPlane = ResourceManager::get()->getFarPlane();
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, nearPlane, farPlane);
    std::vector<glm::mat4> shadowTransforms;
    shadowTransforms.push_back(shadowProj * glm::lookAt(pointLightPos, pointLightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(pointLightPos, pointLightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(pointLightPos, pointLightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(pointLightPos, pointLightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(pointLightPos, pointLightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(pointLightPos, pointLightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    for (unsigned i = 0; i < 6; ++i) {
        this->shader->setMat4("lightShadowMatrices[" + to_string(i) + "]", shadowTransforms[i]);
    }
}
void LightInjectionPass::Render()
{
    // bind FBO
    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
    // use shader
    this->shader->use();
    // clear buffer [!important]
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // set viewport
    glViewport(0, 0, INJECTION_WIDTH, INJECTION_HEIGHT);
    //glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LEQUAL);
    glDisable(GL_DEPTH_TEST);

    // image texture
    unsigned gridTextureR0 = ResourceManager::get()->getTexture("gridTextureR0");
    unsigned gridTextureR1 = ResourceManager::get()->getTexture("gridTextureR1");
    unsigned gridTextureG0 = ResourceManager::get()->getTexture("gridTextureG0");
    unsigned gridTextureG1 = ResourceManager::get()->getTexture("gridTextureG1");
    unsigned gridTextureB0 = ResourceManager::get()->getTexture("gridTextureB0");
    unsigned gridTextureB1 = ResourceManager::get()->getTexture("gridTextureB1");
    //unsigned samplesIdxInGridTexture = ResourceManager::get()->getTexture("samplesIdxInGridTexture");
    // 清空image缓存 samplesIdxInGridTexture不是增量计算，不用清空
    glClearTexImage(gridTextureR0, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
    glClearTexImage(gridTextureR1, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
    glClearTexImage(gridTextureG0, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
    glClearTexImage(gridTextureG1, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
    glClearTexImage(gridTextureB0, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
    glClearTexImage(gridTextureB1, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
    //glClearTexImage(samplesIdxInGridTexture, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, NULL);
    // bind image texture
    glBindImageTexture(0, gridTextureR0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(1, gridTextureR1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(2, gridTextureG0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(3, gridTextureG1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(4, gridTextureB0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(5, gridTextureB1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    //glBindImageTexture(6, samplesIdxInGridTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32UI);
    //cout << "glBindImageTexture " << glGetError() << endl;
    // 绑定输入的纹理
    unsigned shadowWorldPosMap = ResourceManager::get()->getTexture("shadowWorldPosMap");
    this->shader->setInt("shadowWorldPosMap", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, shadowWorldPosMap);

    unsigned shadowFluxMap = ResourceManager::get()->getTexture("shadowFluxMap");
    this->shader->setInt("shadowFluxMap", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, shadowFluxMap);
    //cout << "glBindTexture " << glGetError() << endl;

    // TODO: 绑定cubemap vap
    auto VAOPair = ResourceManager::get()->getVAO("injectionCubemapVAO");
    glBindVertexArray(VAOPair.first);
    //cout << "VAO id = " << VAOPair.first << endl;
    glDrawArrays(GL_TRIANGLES, 0, 36);
    //cout << "TEST Render glDrawArrays " << glGetError() << endl;

    glFinish();

    // get test data from gridTextureR0
    //glBindTexture(GL_TEXTURE_3D, ResourceManager::get()->getTexture("gridTextureR0"));
    //unsigned* testDataR0 = new unsigned[this->iGridTextureSize * this->iGridTextureSize * this->iGridTextureSize];
    //glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, testDataR0);
    //glBindTexture(GL_TEXTURE_3D, ResourceManager::get()->getTexture("gridTextureR1"));
    //unsigned* testDataR1 = new unsigned[this->iGridTextureSize * this->iGridTextureSize * this->iGridTextureSize];
    //glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, testDataR1);
    //glBindTexture(GL_TEXTURE_3D, ResourceManager::get()->getTexture("gridTextureG0"));
    //unsigned* testDataG0 = new unsigned[this->iGridTextureSize * this->iGridTextureSize * this->iGridTextureSize];
    //glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, testDataG0);
    //glBindTexture(GL_TEXTURE_3D, ResourceManager::get()->getTexture("gridTextureG1"));
    //unsigned* testDataG1 = new unsigned[this->iGridTextureSize * this->iGridTextureSize * this->iGridTextureSize];
    //glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, testDataG1);
    //glBindTexture(GL_TEXTURE_3D, ResourceManager::get()->getTexture("gridTextureB0"));
    //unsigned* testDataB0 = new unsigned[this->iGridTextureSize * this->iGridTextureSize * this->iGridTextureSize];
    //glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, testDataB0);
    //glBindTexture(GL_TEXTURE_3D, ResourceManager::get()->getTexture("gridTextureB1"));
    //unsigned* testDataB1 = new unsigned[this->iGridTextureSize * this->iGridTextureSize * this->iGridTextureSize];
    //glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, testDataB1);
    //cout << "TEST DATA gridTextureR0 R1 G0 G1 B0 B1" << glGetError() << endl;
    //for (int i = 0; i < this->iGridTextureSize; ++i) {
    //    for (int j = 0; j < this->iGridTextureSize; ++j) {
    //        for (int k = 0; k < this->iGridTextureSize; ++k) {
    //            int index = i * this->iGridTextureSize * this->iGridTextureSize + j * this->iGridTextureSize + k;
    //            if (testDataR0[index] == 0 &&
    //                testDataR1[index] == 0 &&
    //                testDataG0[index] == 0 &&
    //                testDataG1[index] == 0 &&
    //                testDataB0[index] == 0 &&
    //                testDataB1[index] == 0) continue;
    //            vec2 testR0 = atomToVec2(testDataR0[index]);
    //            vec2 testR1 = atomToVec2(testDataR1[index]);
    //            vec2 testG0 = atomToVec2(testDataG0[index]);
    //            vec2 testG1 = atomToVec2(testDataG1[index]);
    //            vec2 testB0 = atomToVec2(testDataB0[index]);
    //            vec2 testB1 = atomToVec2(testDataB1[index]);
    //            //cout << "grid " << i << "," << j << "," << k;
    //            //cout << " R " << testR0.x << "," << testR0.y << ',' << testR1.x << "," << testR1.y;
    //            //cout << " G " << testG0.x << "," << testG0.y << ',' << testG1.x << "," << testG1.y;
    //            //cout << " B " << testB0.x << "," << testB0.y << ',' << testB1.x << "," << testB1.y << endl;
    //        }
    //    }
    //}
    //cout << endl;
    
    // get test data from samplesIdxInGridTexture
    //glBindTexture(GL_TEXTURE_1D, ResourceManager::get()->getTexture("samplesIdxInGridTexture"));
    //unsigned* testData2 = new unsigned[25 * 4];
    //glGetTexImage(GL_TEXTURE_1D, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, testData2);
    //cout << "TEST DATA samplesIdxInGridTexture " << glGetError() << endl;
    //for (int i = 0; i < 50; ++i) {
    //    cout << testData2[i * 4 + 0] << ", ";
    //    cout << testData2[i * 4 + 1] << ", ";
    //    cout << testData2[i * 4 + 2] << ", ";
    //    cout << testData2[i * 4 + 3] <<endl;
    //}
    //cout << endl;
    

    glBindVertexArray(0);
    // clear FBO [!important]
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned LightInjectionPass::getIGridTextureSize()
{
    return this->iGridTextureSize;
}

void LightPropogationPass::initGlobalSettings()
{
}

void LightPropogationPass::initShader()
{
    this->shader = new Shader("lightPropogation.vert", "lightPropogation.frag");
    this->shader->use();
}

void LightPropogationPass::initTexture()
{
    glGenFramebuffers(1, &this->FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
    unsigned gridTextureR0 = ResourceManager::get()->getTexture("gridTextureR0");
    unsigned gridTextureR1 = ResourceManager::get()->getTexture("gridTextureR1");
    unsigned gridTextureG0 = ResourceManager::get()->getTexture("gridTextureG0");
    unsigned gridTextureG1 = ResourceManager::get()->getTexture("gridTextureG1");
    unsigned gridTextureB0 = ResourceManager::get()->getTexture("gridTextureB0");
    unsigned gridTextureB1 = ResourceManager::get()->getTexture("gridTextureB1");
    this->shader->setInt("gridTextureR0", 0);
    this->shader->setInt("gridTextureR1", 1);
    this->shader->setInt("gridTextureG0", 2);
    this->shader->setInt("gridTextureG1", 3);
    this->shader->setInt("gridTextureB0", 4);
    this->shader->setInt("gridTextureB1", 5);
    unsigned depthMap;
    createTexture2DNull(depthMap, SHADOW_WIDTH, SHADOW_HEIGHT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!: status=" << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void LightPropogationPass::initScene()
{
    auto pResourceManager = ResourceManager::get();

    // screen VAO
    unsigned gridVAO;
    glGenVertexArrays(1, &gridVAO);
    pResourceManager->setVAO("gridVAO", gridVAO, mat4(1.f));

    cout << "LightPropogationPass initScene VAO" << glGetError() << ", " << gridVAO << endl;

    this->shader->setInt("iGridTextureSize", int(this->iGridTextureSize));
    this->shader->setVec3("fGridSize", pResourceManager->getGlobalVec3("fGridSize"));
    this->shader->setVec3("gridMinBox", pResourceManager->getGlobalVec3("boundingVolumeMin"));
}
void LightPropogationPass::Render()
{
    // bind FBO
    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
    // use shader
    this->shader->use();
    // clear buffer [!important]
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    // set viewport
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    unsigned gridTextureR0 = ResourceManager::get()->getTexture("gridTextureR0");
    unsigned gridTextureR1 = ResourceManager::get()->getTexture("gridTextureR1");
    unsigned gridTextureG0 = ResourceManager::get()->getTexture("gridTextureG0");
    unsigned gridTextureG1 = ResourceManager::get()->getTexture("gridTextureG1");
    unsigned gridTextureB0 = ResourceManager::get()->getTexture("gridTextureB0");
    unsigned gridTextureB1 = ResourceManager::get()->getTexture("gridTextureB1");
    // bind image texture
    glBindImageTexture(0, gridTextureR0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(1, gridTextureR1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(2, gridTextureG0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(3, gridTextureG1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(4, gridTextureB0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(5, gridTextureB1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

    //cout << "glBindImageTexture " << glGetError() << endl;
    for (int count = 0; count < this->propogationCount; count++) {
        //unsigned VAO = this->getGridVAO();
        auto VAOPair = ResourceManager::get()->getVAO("gridVAO");
        // testTextureDebug
        //GLuint testTextureUint;
        //glGenTextures(1, &testTextureUint);
        //glBindTexture(GL_TEXTURE_1D, testTextureUint);
        //glTexStorage1D(GL_TEXTURE_1D, 1, GL_RGBA32UI, this->iGridTextureSize * this->iGridTextureSize * this->iGridTextureSize);
        //glBindTexture(GL_TEXTURE_1D, 0);
        //this->shader->setInt("testTextureUint", 6);
        //// test texture float
        //GLuint testTextureFloat;
        //glGenTextures(1, &testTextureFloat);
        //glBindTexture(GL_TEXTURE_1D, testTextureFloat);
        //glTexStorage1D(GL_TEXTURE_1D, 1, GL_RGBA32F, this->iGridTextureSize * this->iGridTextureSize * this->iGridTextureSize);
        //cout << "TEST LightPropogationPass glTexStorage1D " << glGetError() << endl;
        //glBindTexture(GL_TEXTURE_1D, 0);
        //this->shader->setInt("testTextureFloat", 7);
        //glBindImageTexture(6, testTextureUint, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32UI);
        //glBindImageTexture(7, testTextureFloat, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        
        // bind VAO
        // get grid index VAO from
        glBindVertexArray(VAOPair.first);
        //cout << "TEST LightPropogationPass glBindVertexArray " << glGetError() << ", " << VAOPair.first << endl;
        // draw
        glDrawArrays(GL_POINTS, 0, this->iGridTextureSize * this->iGridTextureSize * this->iGridTextureSize);
        //cout << "TEST LightPropogationPass glDrawArrays " << glGetError() << ", " << this->iGridTextureSize * this->iGridTextureSize * this->iGridTextureSize << endl;

        //// testTextureDebug
        //// get test data from testTextureUint
        //glBindTexture(GL_TEXTURE_1D, testTextureUint);
        //unsigned* testData = new unsigned[this->iGridTextureSize * this->iGridTextureSize * this->iGridTextureSize * 4];
        //glGetTexImage(GL_TEXTURE_1D, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, testData);
        //cout << "TEST DATA testTextureUint " << glGetError() << endl;
        //for (int i = 0; i < 20; ++i) {
        //    cout << testData[i * 4 + 0] << ", ";
        //    cout << testData[i * 4 + 1] << ", ";
        //    cout << testData[i * 4 + 2] << ", ";
        //    cout << testData[i * 4 + 3] << endl;
        //}
        //cout << endl;
        //// get test data from testTextureFloat
        //glBindTexture(GL_TEXTURE_1D, testTextureFloat);
        //float* testData2 = new float[this->iGridTextureSize * this->iGridTextureSize * this->iGridTextureSize * 4];
        //glGetTexImage(GL_TEXTURE_1D, 0, GL_RGBA, GL_FLOAT, testData2);
        //cout << "TEST DATA testTextureFloat " << glGetError() << endl;
        //for (int i = 0; i < 20; ++i) {
        //    cout << testData2[i * 4 + 0] << ", ";
        //    cout << testData2[i * 4 + 1] << ", ";
        //    cout << testData2[i * 4 + 2] << ", ";
        //    cout << testData2[i * 4 + 3] << endl;
        //}
        //cout << endl;
        
        //get test data from gridTextureR0
        //glBindTexture(GL_TEXTURE_3D, ResourceManager::get()->getTexture("gridTextureR0"));
        //unsigned* testDataR0 = new unsigned[this->iGridTextureSize * this->iGridTextureSize * this->iGridTextureSize];
        //glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, testDataR0);
        //glBindTexture(GL_TEXTURE_3D, ResourceManager::get()->getTexture("gridTextureR1"));
        //unsigned* testDataR1 = new unsigned[this->iGridTextureSize * this->iGridTextureSize * this->iGridTextureSize];
        //glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, testDataR1);
        //glBindTexture(GL_TEXTURE_3D, ResourceManager::get()->getTexture("gridTextureG0"));
        //unsigned* testDataG0 = new unsigned[this->iGridTextureSize * this->iGridTextureSize * this->iGridTextureSize];
        //glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, testDataG0);
        //glBindTexture(GL_TEXTURE_3D, ResourceManager::get()->getTexture("gridTextureG1"));
        //unsigned* testDataG1 = new unsigned[this->iGridTextureSize * this->iGridTextureSize * this->iGridTextureSize];
        //glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, testDataG1);
        //glBindTexture(GL_TEXTURE_3D, ResourceManager::get()->getTexture("gridTextureB0"));
        //unsigned* testDataB0 = new unsigned[this->iGridTextureSize * this->iGridTextureSize * this->iGridTextureSize];
        //glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, testDataB0);
        //glBindTexture(GL_TEXTURE_3D, ResourceManager::get()->getTexture("gridTextureB1"));
        //unsigned* testDataB1 = new unsigned[this->iGridTextureSize * this->iGridTextureSize * this->iGridTextureSize];
        //glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, testDataB1);
        //cout << "TEST DATA gridTextureR0 R1 G0 G1 B0 B1" << glGetError() << endl;
        //for (int i = 0; i < this->iGridTextureSize; ++i) {
        //    for (int j = 0; j < this->iGridTextureSize; ++j) {
        //        for (int k = 0; k < this->iGridTextureSize; ++k) {
        //            int index = i * this->iGridTextureSize * this->iGridTextureSize + j * this->iGridTextureSize + k;
        //            if (testDataR0[index] == 0 &&
        //                testDataR1[index] == 0 &&
        //                testDataG0[index] == 0 &&
        //                testDataG1[index] == 0 &&
        //                testDataB0[index] == 0 &&
        //                testDataB1[index] == 0) continue;
        //            vec2 testR0 = atomToVec2(testDataR0[index]);
        //            vec2 testR1 = atomToVec2(testDataR1[index]);
        //            vec2 testG0 = atomToVec2(testDataG0[index]);
        //            vec2 testG1 = atomToVec2(testDataG1[index]);
        //            vec2 testB0 = atomToVec2(testDataB0[index]);
        //            vec2 testB1 = atomToVec2(testDataB1[index]);
        //            //cout << "grid " << i << "," << j << "," << k;
        //            //cout << " R " << testR0.x << "," << testR0.y << ',' << testR1.x << "," << testR1.y;
        //            //cout << " G " << testG0.x << "," << testG0.y << ',' << testG1.x << "," << testG1.y;
        //            //cout << " B " << testB0.x << "," << testB0.y << ',' << testB1.x << "," << testB1.y << endl;
        //        }
        //    }
        //}
        //cout << endl;
    }

    // clear VAO
    glBindVertexArray(0);
    // clear FBO [!important]
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GBufferPass::initGlobalSettings()
{
}

void GBufferPass::initShader()
{
    this->shader = new Shader("GBuffer.vert", "GBuffer.frag");
    this->shader->use();
}

void GBufferPass::initTexture()
{
    glGenFramebuffers(1, &this->FBO);
    //this->FBO = 0;
    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
    unsigned screenDepthMap, screenWorldPosMap, screenDiffuseMap, screenNormalMap;
    // texture
    createTexture2DNull(screenDepthMap, SHADOW_WIDTH, SHADOW_HEIGHT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);
    createTexture2DNull(screenWorldPosMap, SHADOW_WIDTH, SHADOW_HEIGHT, GL_RGB32F, GL_RGB, GL_FLOAT);
    createTexture2DNull(screenDiffuseMap, SHADOW_WIDTH, SHADOW_HEIGHT, GL_RGB32F, GL_RGB, GL_FLOAT);
    createTexture2DNull(screenNormalMap, SHADOW_WIDTH, SHADOW_HEIGHT, GL_RGB32F, GL_RGB, GL_FLOAT);


    ResourceManager::get()->setTexture("screenWorldPosMap", screenWorldPosMap);
    ResourceManager::get()->setTexture("screenDiffuseMap", screenDiffuseMap);
    ResourceManager::get()->setTexture("screenNormalMap", screenNormalMap);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, screenDepthMap, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, screenWorldPosMap, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, screenDiffuseMap, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, screenNormalMap, 0);
    //cout << "GBufferPass glFramebufferTexture " << glGetError() << endl;
    GLenum bufs[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, bufs);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!: status=" << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GBufferPass::initScene()
{
}

void GBufferPass::Render()
{
    auto pRM = ResourceManager::get();
    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
    this->shader->use();

    // !important
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

    pair<Model*, mat4> modelPair = pRM->getModel("renderModel");
    this->shader->setMat4("model", modelPair.second);
    auto camera = pRM->getCamera();
    this->shader->setMat4("view", camera->GetViewMatrix());
    this->shader->setMat4("projection", glm::perspective(glm::radians(camera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f));
    modelPair.first->Draw(*this->shader);
    //cout << "GBufferPass drawModel " << glGetError() << endl;

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
    //glDepthFunc(GL_LESS);
    glDepthFunc(GL_LEQUAL);
    this->shader->use();

    mat4 view = mat4(mat3(ResourceManager::get()->getCamera()->GetViewMatrix())); // remove translation from the view matrix
    mat4 projection = perspective(radians(ResourceManager::get()->getCamera()->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    this->shader->setMat4("view", view);
    this->shader->setMat4("projection", projection);

    //glDisable(GL_DEPTH_TEST);
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
    //cout << "Output2DPass initScene " << glGetError() << endl;
    ResourceManager::get()->setVAO("screenVAO", screenVAO, mat4(1.f));
    glBindVertexArray(0);
}
void Output2DPass::Render()
{
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    this->shader->use();
    pair<unsigned, mat4> VAOPair = ResourceManager::get()->getVAO("screenVAO");
    glBindVertexArray(VAOPair.first);
    glActiveTexture(GL_TEXTURE0);
    unsigned screenTexture = ResourceManager::get()->getTexture(this->textureName);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    //cout << "Output2DPass glDrawArrays " << glGetError() << endl;
    glBindVertexArray(0);
}

void LPVOutputPass::initGlobalSettings()
{
}

void LPVOutputPass::initShader()
{
    this->shader = new Shader("LPVOutput.vert", "LPVOutput.frag");
    this->shader->use();
}

void LPVOutputPass::initTexture()
{
    auto pRM = ResourceManager::get();
    // shader set
    this->shader->setVec3("light.position", pRM->getPointLightPos());
    this->shader->setVec3("light.diffuse", pRM->getPointLightDiffuse());
    this->shader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);
    this->shader->setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
    this->shader->setFloat("farPlane", pRM->getFarPlane());

    this->shader->setInt("gridTextureR0", 0);
    this->shader->setInt("gridTextureR1", 1);
    this->shader->setInt("gridTextureG0", 2);
    this->shader->setInt("gridTextureG1", 3);
    this->shader->setInt("gridTextureB0", 4);
    this->shader->setInt("gridTextureB1", 5);
    this->shader->setInt("shadowWorldPosMap", 0);
    this->shader->setInt("screenWorldPosMap", 1);
    this->shader->setInt("screenDiffuseMap", 2);
    this->shader->setInt("screenNormalMap", 3);

    this->shader->setInt("iGridTextureSize", this->iGridTextureSize);
    this->shader->setVec3("fGridSize", pRM->getGlobalVec3("fGridSize"));
    this->shader->setVec3("gridMinBox", pRM->getGlobalVec3("boundingVolumeMin"));
}

void LPVOutputPass::initScene()
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
    //cout << "LPVOutputPass initScene " << glGetError() << endl;
    ResourceManager::get()->setVAO("screenVAO", screenVAO, mat4(1.f));
    glBindVertexArray(0);
}

void LPVOutputPass::Render()
{
    auto pRM = ResourceManager::get();
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    this->shader->use();

    this->shader->setVec3("viewPos", pRM->getCamera()->Position);

    glActiveTexture(GL_TEXTURE0);
    unsigned shadowWorldPosMap = pRM->getTexture("shadowWorldPosMap");
    glBindTexture(GL_TEXTURE_CUBE_MAP, shadowWorldPosMap);

    glActiveTexture(GL_TEXTURE1);
    unsigned screenWorldPosMap = pRM->getTexture("screenWorldPosMap");
    glBindTexture(GL_TEXTURE_2D, screenWorldPosMap);
    glActiveTexture(GL_TEXTURE2);
    unsigned screenDiffuseMap = pRM->getTexture("screenDiffuseMap");
    glBindTexture(GL_TEXTURE_2D, screenDiffuseMap);
    glActiveTexture(GL_TEXTURE3);
    unsigned screenNormalMap = pRM->getTexture("screenNormalMap");
    glBindTexture(GL_TEXTURE_2D, screenNormalMap);

    unsigned gridTextureR0 = pRM->getTexture("gridTextureR0");
    unsigned gridTextureR1 = pRM->getTexture("gridTextureR1");
    unsigned gridTextureG0 = pRM->getTexture("gridTextureG0");
    unsigned gridTextureG1 = pRM->getTexture("gridTextureG1");
    unsigned gridTextureB0 = pRM->getTexture("gridTextureB0");
    unsigned gridTextureB1 = pRM->getTexture("gridTextureB1");
    // bind image texture
    glBindImageTexture(0, gridTextureR0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(1, gridTextureR1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(2, gridTextureG0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(3, gridTextureG1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(4, gridTextureB0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(5, gridTextureB1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    //cout << "LPVOutputPass glBindImageTexture " << glGetError() << endl;

    pair<unsigned, mat4> VAOPair = pRM->getVAO("screenVAO");
    glBindVertexArray(VAOPair.first);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    //cout << "LPVOutputPass glDrawArrays " << glGetError() << endl;
    glBindVertexArray(0);
}
