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

    //int NumberOfExtensions;
    //glGetIntegerv(GL_NUM_EXTENSIONS, &NumberOfExtensions);
    //for (int i = 0; i < NumberOfExtensions; i++) {
    //    const GLubyte* ccc = glGetStringi(GL_EXTENSIONS, i);
    //    cout << ccc << endl;
    //}
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
    createTextureCubeMapNull(shadowDepthMap, SHADOW_WIDTH, SHADOW_HEIGHT, GL_DEPTH_COMPONENT);
	createTextureCubeMapNull(worldPosMap, SHADOW_WIDTH, SHADOW_HEIGHT, GL_RGB);
	createTextureCubeMapNull(fluxMap, SHADOW_WIDTH, SHADOW_HEIGHT, GL_RGB);

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
    //Model* nanosuit = new Model("../models/Room/Room #1.obj");
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
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

    pair<Model*, mat4> modelPair = ResourceManager::get()->getModel("nanosuit");
    this->shader->setMat4("model", modelPair.second);
    modelPair.first->Draw(*this->shader);

    // !important
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// @return: float数组，长度3*samplesN，数据大小[0, 1]
float* LightInjectionPass::getSamplesRandom(unsigned samplesN)
{
    // TODO: 对球面均匀采样
    float* sampleRes = new float[samplesN * 3];
    RandomGenerator randomGenerator;
    vector<vec3> samples3D = randomGenerator.uniform0To1By3D(samplesN);
    for (int i = 0; i < samples3D.size(); ++i) {
        if (i * 3 + 2 >= int(samplesN) * 3) {
            cout << "ERROR: buffer overflow in getSamplesRandom(), samples array size="<< samples3D.size() << endl;
            return nullptr;
        }
        vec3 sample = samples3D[i];
        sampleRes[i * 3 + 0] = 1.f - 2.f * sample.x;
        sampleRes[i * 3 + 1] = 1.f - 2.f * sample.y;
        sampleRes[i * 3 + 2] = 1.f - 2.f * sample.z;
    }
    return sampleRes;
}

void LightInjectionPass::initGlobalSettings()
{
}

void LightInjectionPass::initShader()
{
    this->shader = new Shader("lightInjection.vert", "lightInjection.frag");
    this->shader->use();
}

void LightInjectionPass::initTexture()
{
    glGenFramebuffers(1, &this->FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
    // TODO: 以体面的方式划分格子，在第一个pass中/CPU找出每个轴的最小和最大值
    const vec3 gridMaxBox = { 50.f, 50.f, 50.f };
    const vec3 gridMinBox = { -50.f, -50.f, -50.f };
    const vec3 fGridSize = (gridMaxBox - gridMinBox) / float(this->uGridTextureSize);
    this->shader->setVec3("gridSize", fGridSize);
    this->shader->setVec3("gridMinBox", gridMinBox);

    unsigned gridTextureR, gridTextureG, gridTextureB;
    glGenTextures(1, &gridTextureR);
    glBindTexture(GL_TEXTURE_3D, gridTextureR);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, this->uGridTextureSize, this->uGridTextureSize, this->uGridTextureSize);
    //glTexImage3D(GL_TEXTURE_3D, 0, GL_R32UI, this->uGridTextureSize, this->uGridTextureSize, this->uGridTextureSize, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
    cout << "glTexImage3D " << glGetError() << endl;

    glGenTextures(1, &gridTextureG);
    glBindTexture(GL_TEXTURE_3D, gridTextureG);
    //glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, this->uGridTextureSize, this->uGridTextureSize, this->uGridTextureSize);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32UI, this->uGridTextureSize, this->uGridTextureSize, this->uGridTextureSize, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);

    glGenTextures(1, &gridTextureB);
    glBindTexture(GL_TEXTURE_3D, gridTextureB);
    //glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, this->uGridTextureSize, this->uGridTextureSize, this->uGridTextureSize);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32UI, this->uGridTextureSize, this->uGridTextureSize, this->uGridTextureSize, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
    glBindTexture(GL_TEXTURE_3D, 0);

    glBindImageTexture(0, gridTextureR, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    cout << "glBindImageTexture " << glGetError() << endl;
    glBindImageTexture(1, gridTextureG, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(2, gridTextureB, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

    ResourceManager::get()->setTexture("girdTextureR", gridTextureR);
    ResourceManager::get()->setTexture("girdTextureG", gridTextureG);
    ResourceManager::get()->setTexture("girdTextureB", gridTextureB);

    this->shader->setInt("girdTextureR", 0);
    this->shader->setInt("girdTextureG", 1);
    this->shader->setInt("girdTextureB", 2);

    // test texture
    GLuint testTexture;
    glGenTextures(1, &testTexture);
    glBindTexture(GL_TEXTURE_1D, testTexture);
    glTexStorage1D(GL_TEXTURE_1D, 1, GL_RGBA32F, this->uGridTextureSize);
    glBindTexture(GL_TEXTURE_1D, 0);
    glBindImageTexture(3, testTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    this->shader->setInt("testTexture", 3);
    ResourceManager::get()->setTexture("testTexture", testTexture);

    unsigned depthMap;
    createTexture2DNull(depthMap, SHADOW_WIDTH, SHADOW_HEIGHT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);

    //unsigned colorBufferMap;
    //createTexture2DNull(colorBufferMap, SHADOW_WIDTH, SHADOW_HEIGHT, GL_RGB16F, GL_RGB, GL_FLOAT);
    //glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, colorBufferMap, 0);
    //GLenum bufs[1] = { GL_COLOR_ATTACHMENT0 };
    //glDrawBuffers(1, bufs);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!: status=" << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void LightInjectionPass::initScene()
{

}

void LightInjectionPass::Render()
{
    // bind FBO
    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
    // use shader
    this->shader->use();
    // clear buffer [!important]
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glDisable(GL_DEPTH_TEST);
    // set viewport
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    // 清空image缓存
    glClearTexImage(ResourceManager::get()->getTexture("girdTextureR"), 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
    glClearTexImage(ResourceManager::get()->getTexture("girdTextureG"), 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
    glClearTexImage(ResourceManager::get()->getTexture("girdTextureB"), 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
    //cout << "glClearTexImage " << glGetError() << endl;

    // 绑定输入的纹理
    unsigned worldPosMap = ResourceManager::get()->getTexture("worldPosMap");
    this->shader->setInt("worldPosMap", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, worldPosMap);
    //cout << "glBindTexture worldPosMap " << glGetError() << endl;

    unsigned fluxMap = ResourceManager::get()->getTexture("fluxMap");
    this->shader->setInt("fluxMap", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, fluxMap);
    //cout << "glBindTexture fluxMap " << glGetError() << endl;

    // 传递的数据VAO
    // TODO: DELETE VAO
    float* vertices = this->getSamplesRandom(this->samplesN);
    //vertices[0] = -0.7592f;
    //vertices[1] = -0.389f;
    //vertices[2] = -0.6848f;
    vertices[0] = -0.5898f;
    vertices[1] = -0.0318f;
    vertices[2] = -0.6212f;
    if (vertices == nullptr) {
        cout << "ERROR: failed to get samples in getSamplesLHS" << endl;
    }
    for (int i = 0; i < this->samplesN; ++i) {
        cout << "sample: x, y, z = " << vertices[i * 3 + 0] << ", " << vertices[i * 3 + 1] << ", " << vertices[i * 3 + 2] << endl;
    }
    unsigned sampleVAO, sampleVBO;
    glGenVertexArrays(1, &sampleVAO);
    glGenBuffers(1, &sampleVBO);
    glBindVertexArray(sampleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sampleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * this->samplesN, vertices, GL_STATIC_DRAW);
    //cout << "TEST VBO " << glGetError() << endl;
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //cout << "TEST VAO " << glGetError() << endl;
    glBindVertexArray(0);
    ResourceManager::get()->setVAO("sampleVAO", sampleVAO, mat4(1.f));

    pair<unsigned, mat4> VAOPair = ResourceManager::get()->getVAO("sampleVAO");
    glBindVertexArray(VAOPair.first);
    glDrawArrays(GL_POINTS, 0, this->samplesN);
    //cout << "glDrawArrays " << glGetError() << endl;

    glFinish();

    // get test data from girdTextureR
    glBindTexture(GL_TEXTURE_3D, ResourceManager::get()->getTexture("girdTextureR"));
    unsigned* testData = new unsigned[this->uGridTextureSize * this->uGridTextureSize * this->uGridTextureSize];
    glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, testData);
    cout << "TEST DATA girdTextureR " << glGetError() << endl;
    cout << "                                          ";
    bool flag = false;
    for (int i = 0; i < this->uGridTextureSize * this->uGridTextureSize * this->uGridTextureSize; ++i) {
        cout << testData[i] << " ";
        //if (testData[i] != 282) flag = true;
    }
    cout << endl;

    // get test data from testTexture
    glBindTexture(GL_TEXTURE_1D, ResourceManager::get()->getTexture("testTexture"));
    float* testData2 = new float[this->uGridTextureSize * 4];
    glGetTexImage(GL_TEXTURE_1D, 0, GL_RGBA, GL_FLOAT, testData2);
    cout << "TEST DATA testTexture " << glGetError() << endl;
    //cout << "                                   ";
    for (int i = 0; i < this->uGridTextureSize * 4; ++i) {
        cout << testData2[i] << " ";
        if (testData2[i] < 0.99f && testData2[i] > 0.1f) flag = true;
    }
    cout << endl;
    if (flag) system("pause");

    glBindVertexArray(0);
    // clear FBO [!important]
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