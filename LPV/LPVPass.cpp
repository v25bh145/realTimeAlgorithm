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

// TODO: GL_MAX_IMAGE_UNITS judge

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

// @return: float数组，长度3*samplesN，数据大小[0, 1]
float* LightInjectionPass::getSamplesRandom(unsigned samplesN)
{
    vec3 lightPos = ResourceManager::get()->getPointLightPos();
    pair<Model*, mat4> modelPair = ResourceManager::get()->getModel("renderModel");
    int numsOfVertices = 0;
    for (Mesh mesh : modelPair.first->meshes) {
        numsOfVertices += mesh.vertices.size();
    }
    float* samplesRes = new float[samplesN * 4];
    RandomGenerator randomGenerator;
    vector<int> samplesIndexArray;
    for (int i = 0; i < samplesN; ++i) {
        samplesIndexArray.push_back(randomGenerator.uniformNToM(0, numsOfVertices));
    }
    sort(samplesIndexArray.begin(), samplesIndexArray.end());

    //cout << "debug-info samplesIndexArray" << endl;
    //for (int i = 0; i < samplesN; ++i) {
    //    cout << samplesIndexArray[i] << ", ";
    //}
    //cout << endl;

    int currentVertices = 0;
    int currentSampleIndex = 0;
    for (Mesh mesh : modelPair.first->meshes) {
        //cout << "debug-info: current vertices=" << currentVertices << ", current sample index=" << currentVertices << ", current sample vertice=" << samplesIndexArray[currentSampleIndex] << endl;
        while (currentSampleIndex < samplesIndexArray.size() && samplesIndexArray[currentSampleIndex] < currentVertices + mesh.vertices.size()) {
            if (currentSampleIndex * 4 + 3 >= int(samplesN) * 4) {
                cout << "ERROR: buffer overflow in getSamplesRandom(), samples array size="<< currentSampleIndex << endl;
                return nullptr;
            }
            samplesRes[currentSampleIndex * 4 + 0] = float(mesh.vertices[samplesIndexArray[currentSampleIndex] - currentVertices].Position.x) - lightPos.x;
            samplesRes[currentSampleIndex * 4 + 1] = float(mesh.vertices[samplesIndexArray[currentSampleIndex] - currentVertices].Position.y) - lightPos.y;
            samplesRes[currentSampleIndex * 4 + 2] = float(mesh.vertices[samplesIndexArray[currentSampleIndex] - currentVertices].Position.z) - lightPos.z;
            samplesRes[currentSampleIndex * 4 + 3] = float(currentSampleIndex);
            currentSampleIndex++;
        }
        if (currentSampleIndex == samplesIndexArray.size()) break;
        currentVertices += mesh.vertices.size();
    }
    return samplesRes;
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
    auto pResourceManager = ResourceManager::get();
    vec3 gridMaxBox = pResourceManager->getGlobalVec3("boundingVolumeMax");
    vec3 gridMinBox = pResourceManager->getGlobalVec3("boundingVolumeMin");
    cout << "maxBoundingBox = " << gridMaxBox.x << ", " << gridMaxBox.y << "," << gridMaxBox.z << endl;
    cout << "minBoundingBox = " << gridMinBox.x << ", " << gridMinBox.y << "," << gridMinBox.z << endl;

    const vec3 fGridSize = (gridMaxBox - gridMinBox) / float(this->uGridTextureSize);
    pResourceManager->setGlobalVec3("fGridSize", fGridSize);
    this->shader->setVec3("gridSize", fGridSize);
    this->shader->setVec3("gridMinBox", gridMinBox);

    unsigned gridTextureR0, gridTextureG0, gridTextureB0, gridTextureR1, gridTextureG1, gridTextureB1;
    glGenTextures(1, &gridTextureR0);
    glBindTexture(GL_TEXTURE_3D, gridTextureR0);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, this->uGridTextureSize, this->uGridTextureSize, this->uGridTextureSize);
    //glTexImage3D(GL_TEXTURE_3D, 0, GL_R32UI, this->uGridTextureSize, this->uGridTextureSize, this->uGridTextureSize, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
    cout << "glTexImage3D " << glGetError() << endl;
    glGenTextures(1, &gridTextureR1);
    glBindTexture(GL_TEXTURE_3D, gridTextureR1);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, this->uGridTextureSize, this->uGridTextureSize, this->uGridTextureSize);

    glGenTextures(1, &gridTextureG0);
    glBindTexture(GL_TEXTURE_3D, gridTextureG0);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, this->uGridTextureSize, this->uGridTextureSize, this->uGridTextureSize);
    glGenTextures(1, &gridTextureG1);
    glBindTexture(GL_TEXTURE_3D, gridTextureG1);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, this->uGridTextureSize, this->uGridTextureSize, this->uGridTextureSize);

    glGenTextures(1, &gridTextureB0);
    glBindTexture(GL_TEXTURE_3D, gridTextureB0);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, this->uGridTextureSize, this->uGridTextureSize, this->uGridTextureSize);
    glGenTextures(1, &gridTextureB1);
    glBindTexture(GL_TEXTURE_3D, gridTextureB1);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, this->uGridTextureSize, this->uGridTextureSize, this->uGridTextureSize);

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
    GLuint samplesIdxInGridTexture;
    glGenTextures(1, &samplesIdxInGridTexture);
    glBindTexture(GL_TEXTURE_1D, samplesIdxInGridTexture);
    glTexStorage1D(GL_TEXTURE_1D, 1, GL_RGBA32UI, this->samplesN);
    glBindTexture(GL_TEXTURE_1D, 0);
    this->shader->setInt("samplesIdxInGridTexture", 6);
    pResourceManager->setTexture("samplesIdxInGridTexture", samplesIdxInGridTexture);

    unsigned depthMap;
    createTexture2DNull(depthMap, SHADOW_WIDTH, SHADOW_HEIGHT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!: status=" << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void LightInjectionPass::initScene()
{

}

void LightInjectionPass::genSampleVAO()
{
    float* vertices = this->getSamplesRandom(this->samplesN);
    //vertices[0] = -0.5898f;
    //vertices[1] = -0.0318f;
    //vertices[2] = -0.6212f;
    if (vertices == nullptr) {
        cout << "ERROR: failed to get samples in getSamplesLHS" << endl;
    }
    for (int i = 0; i < this->samplesN; ++i) {
        cout << "sample: x, y, z, i = " << vertices[i * 4 + 0] << ", " << vertices[i * 4 + 1] << ", " << vertices[i * 4 + 2] << ", " << vertices[i * 4 + 3] << endl;
    }
    unsigned sampleVAO, sampleVBO;
    glGenVertexArrays(1, &sampleVAO);
    glGenBuffers(1, &sampleVBO);
    glBindVertexArray(sampleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sampleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * this->samplesN, vertices, GL_STATIC_DRAW);
    cout << "TEST sampleVBO " << glGetError() << endl;
    // sample tex
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // sample index
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    cout << "TEST sampleVAO " << glGetError() << endl;
    cout << "VAO id = " << sampleVAO << endl;
    glBindVertexArray(0);
    ResourceManager::get()->setVAO("sampleVAO", sampleVAO, mat4(1.f));
}

void LightInjectionPass::Render()
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

    // image texture
    unsigned gridTextureR0 = ResourceManager::get()->getTexture("gridTextureR0");
    unsigned gridTextureR1 = ResourceManager::get()->getTexture("gridTextureR1");
    unsigned gridTextureG0 = ResourceManager::get()->getTexture("gridTextureG0");
    unsigned gridTextureG1 = ResourceManager::get()->getTexture("gridTextureG1");
    unsigned gridTextureB0 = ResourceManager::get()->getTexture("gridTextureB0");
    unsigned gridTextureB1 = ResourceManager::get()->getTexture("gridTextureB1");
    unsigned samplesIdxInGridTexture = ResourceManager::get()->getTexture("samplesIdxInGridTexture");
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
    glBindImageTexture(6, samplesIdxInGridTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32UI);
    cout << "glBindImageTexture " << glGetError() << endl;
    // 绑定输入的纹理
    unsigned worldPosMap = ResourceManager::get()->getTexture("worldPosMap");
    this->shader->setInt("worldPosMap", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, worldPosMap);

    unsigned fluxMap = ResourceManager::get()->getTexture("fluxMap");
    this->shader->setInt("fluxMap", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, fluxMap);

    // 传递的数据VAO
    // TODO: DELETE VAO
    // 可选方案: 在VAO中采样，直接采样几个顶点，如果被遮蔽必定有其他的顶点对上
    this->genSampleVAO();

    pair<unsigned, mat4> VAOPair = ResourceManager::get()->getVAO("sampleVAO");
    glBindVertexArray(VAOPair.first);
    cout << "VAO id = " << VAOPair.first << endl;
    glDrawArrays(GL_POINTS, 0, this->samplesN);
    cout << "TEST Render glDrawArrays " << glGetError() << endl;

    glFinish();

     //get test data from gridTextureR0
    glBindTexture(GL_TEXTURE_3D, ResourceManager::get()->getTexture("gridTextureR0"));
    unsigned* testData = new unsigned[this->uGridTextureSize * this->uGridTextureSize * this->uGridTextureSize];
    glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, testData);
    cout << "TEST DATA gridTextureR0 " << glGetError() << endl;
    for (int i = 0; i < this->uGridTextureSize * this->uGridTextureSize * this->uGridTextureSize; ++i) {
        cout << testData[i] << " ";
    }
    cout << endl;
    
    // get test data from samplesIdxInGridTexture
    glBindTexture(GL_TEXTURE_1D, ResourceManager::get()->getTexture("samplesIdxInGridTexture"));
    unsigned* testData2 = new unsigned[this->samplesN * 4];
    glGetTexImage(GL_TEXTURE_1D, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, testData2);

    cout << "TEST DATA samplesIdxInGridTexture " << glGetError() << endl;
    for (int i = 0; i < this->samplesN; ++i) {
        cout << testData2[i * 4 + 0] << ", ";
        cout << testData2[i * 4 + 1] << ", ";
        cout << testData2[i * 4 + 2] << ", ";
        cout << testData2[i * 4 + 3] <<endl;
    }
    cout << endl;

    glBindVertexArray(0);
    // clear FBO [!important]
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned LightInjectionPass::getSamplesN()
{
    return this->samplesN;
}

unsigned LightInjectionPass::getUGridTextureSize()
{
    return this->uGridTextureSize;
}

bool LightPropogationPass::compareU32vec3(const glm::u32vec3 v1, const glm::u32vec3 v2)
{
    if (v1.x < v2.x || (v1.x == v2.x && (v1.y < v2.y || (v1.y == v2.y && v1.z < v2.z)))) return true;
    return false;
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
    // test texture uint
    GLuint testTextureUint;
    glGenTextures(1, &testTextureUint);
    glBindTexture(GL_TEXTURE_1D, testTextureUint);
    // 实际会使用uniquedPoints大小的空间，不过保证uniquedPoints<=samplesN
    glTexStorage1D(GL_TEXTURE_1D, 1, GL_RGBA32UI, this->samplesN);
    glBindTexture(GL_TEXTURE_1D, 0);
    this->shader->setInt("testTextureUint", 6);
    ResourceManager::get()->setTexture("testTextureUint", testTextureUint);
    // test texture float
    GLuint testTextureFloat;
    glGenTextures(1, &testTextureFloat);
    glBindTexture(GL_TEXTURE_1D, testTextureFloat);
    // 实际会使用uniquedPoints大小的空间，不过保证uniquedPoints<=samplesN
    glTexStorage1D(GL_TEXTURE_1D, 1, GL_RGBA32F, this->samplesN);
    glBindTexture(GL_TEXTURE_1D, 0);
    this->shader->setInt("testTextureFloat", 7);
    ResourceManager::get()->setTexture("testTextureFloat", testTextureFloat);
    unsigned depthMap;
    createTexture2DNull(depthMap, SHADOW_WIDTH, SHADOW_HEIGHT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!: status=" << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void LightPropogationPass::initScene()
{
    auto pResourceManager = ResourceManager::get();
    this->shader->setInt("propogationCount", this->propogationCount);
    this->shader->setFloat("propogationGate", this->propogationGate);

    this->shader->setInt("uGridTextureSize", int(this->uGridTextureSize));
    this->shader->setVec3("fGridSize", pResourceManager->getGlobalVec3("fGridSize"));
    this->shader->setVec3("gridMinBox", pResourceManager->getGlobalVec3("boundingVolumeMin"));
}

unsigned LightPropogationPass::getVAOFromSamplesIdxGridTex()
{
    glBindTexture(GL_TEXTURE_1D, ResourceManager::get()->getTexture("samplesIdxInGridTexture"));
    unsigned* indexData = new unsigned[this->samplesN * 4];
    glGetTexImage(GL_TEXTURE_1D, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, indexData);
    vector<u32vec3> indexArray;
    cout << "TEST DATA samplesIdxInGridTexture in getVAOFromSamplesIdxGridTex " << glGetError() << endl;
    for (int i = 0; i < this->samplesN; ++i) {
        if (indexData[i * 4 + 3]) {
            indexArray.push_back({ indexData[i * 4 + 0] ,indexData[i * 4 + 1] ,indexData[i * 4 + 2] });
        }
        else {
            cout << "debug-info: invalid bit in i = " << i << endl;
        }
    }
    // sort vec3
    sort(indexArray.begin(), indexArray.end(), LightPropogationPass::compareU32vec3);
    // unique
    auto pRes = unique(indexArray.begin(), indexArray.end());
    indexArray.erase(pRes, indexArray.end());
    this->uniquedPoints = indexArray.size();
    float* inputIndexData = new float[indexArray.size() * 4];
    cout << "inputIndexData" << endl;
    for (int i = 0; i < indexArray.size(); ++i) {
        u32vec3 v = indexArray[i];
        inputIndexData[i * 4 + 0] = float(v.x);
        inputIndexData[i * 4 + 1] = float(v.y);
        inputIndexData[i * 4 + 2] = float(v.z);
        inputIndexData[i * 4 + 3] = float(i);
        cout << inputIndexData[i * 4 + 0] << ", " << inputIndexData[i * 4 + 1] << ", " << inputIndexData[i * 4 + 2] << ", " << inputIndexData[i * 4 + 3] << endl;
    }
    unsigned idxVAO, idxVBO;
    glGenVertexArrays(1, &idxVAO);
    glGenBuffers(1, &idxVBO);
    glBindVertexArray(idxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, idxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * indexArray.size(), inputIndexData, GL_STATIC_DRAW);
    cout << "TEST idxVBO " << glGetError() << endl;
    // sample tex
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // sample index
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    cout << "TEST idxVAO " << glGetError() << endl;
    glBindVertexArray(0);
    return idxVAO;
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
    unsigned testTextureUint = ResourceManager::get()->getTexture("testTextureUint");
    unsigned testTextureFloat = ResourceManager::get()->getTexture("testTextureFloat");
    // bind image texture
    glBindImageTexture(0, gridTextureR0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(1, gridTextureR1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(2, gridTextureG0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(3, gridTextureG1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(4, gridTextureB0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(5, gridTextureB1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(6, testTextureUint, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32UI);
    glBindImageTexture(7, testTextureFloat, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    cout << "glBindImageTexture " << glGetError() << endl;
    // bind VAO
    // get grid index VAO from
    unsigned VAO = this->getVAOFromSamplesIdxGridTex();
    glBindVertexArray(VAO);
    cout << "TEST LightPropogationPass glBindVertexArray " << glGetError() << ", " << VAO << endl;
    // draw
    glDrawArrays(GL_POINTS, 0, this->uniquedPoints);
    cout << "TEST LightPropogationPass glDrawArrays " << glGetError() << ", " << this->uniquedPoints << endl;
    glFinish();
    // get test data from testTextureUint
    glBindTexture(GL_TEXTURE_1D, ResourceManager::get()->getTexture("testTextureUint"));
    unsigned* testData = new unsigned[this->samplesN * 4];
    glGetTexImage(GL_TEXTURE_1D, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, testData);
    cout << "TEST DATA testTextureUint " << glGetError() << endl;
    for (int i = 0; i < this->uniquedPoints; ++i) {
        cout << testData[i * 4 + 0] << ", ";
        cout << testData[i * 4 + 1] << ", ";
        cout << testData[i * 4 + 2] << ", ";
        cout << testData[i * 4 + 3] << endl;
    }
    cout << endl;
    // get test data from testTextureFloat
    glBindTexture(GL_TEXTURE_1D, ResourceManager::get()->getTexture("testTextureFloat"));
    float* testData2 = new float[this->samplesN * 4];
    glGetTexImage(GL_TEXTURE_1D, 0, GL_RGBA, GL_FLOAT, testData2);
    cout << "TEST DATA testTextureFloat " << glGetError() << endl;
    for (int i = 0; i < this->uniquedPoints; ++i) {
        cout << testData2[i * 4 + 0] << ", ";
        cout << testData2[i * 4 + 1] << ", ";
        cout << testData2[i * 4 + 2] << ", ";
        cout << testData2[i * 4 + 3] << endl;
    }
    cout << endl;

    //get test data from gridTextureR0
    glBindTexture(GL_TEXTURE_3D, ResourceManager::get()->getTexture("gridTextureR0"));
    unsigned* testData3 = new unsigned[this->uGridTextureSize * this->uGridTextureSize * this->uGridTextureSize];
    glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, testData3);
    cout << "TEST DATA gridTextureR0 " << glGetError() << endl;
    for (int i = 0; i < this->uGridTextureSize * this->uGridTextureSize * this->uGridTextureSize; ++i) {
        cout << testData3[i] << " ";
    }
    cout << endl;

    // clear VAO
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