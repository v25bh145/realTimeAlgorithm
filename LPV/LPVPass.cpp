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
}

void GetShadowSamplePass::initShader()
{
	this->shader = Shader("getShadowSample.vert", "getShadowSample.frag", "getShadowSample.geom");
    this->shader.use();
}
void GetShadowSamplePass::initTexture()
{
	glGenFramebuffers(1, &this->FBO);
    //this->FBO = 0;
	glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
	// texture
    createTextureCubeMapNull(this->shadowDepthMap, GetShadowSamplePass::SHADOW_WIDTH, GetShadowSamplePass::SHADOW_HEIGHT, GL_DEPTH_COMPONENT);
	createTextureCubeMapNull(this->worldPosMap, GetShadowSamplePass::SHADOW_WIDTH, GetShadowSamplePass::SHADOW_HEIGHT, GL_RGB);
	createTextureCubeMapNull(this->fluxMap, GetShadowSamplePass::SHADOW_WIDTH, GetShadowSamplePass::SHADOW_HEIGHT, GL_RGB);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, this->shadowDepthMap, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, this->worldPosMap, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, this->fluxMap, 0);
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
    // light geom
	this->pointLightDiffuse = {1.f, 1.f, 1.f};
	this->pointLightPos = { 2.5f, 2.0f, 2.5f };
    // light transform
    float near_plane = 1.0f;
    float far_plane = 25.0f;
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
    std::vector<glm::mat4> shadowTransforms;
    shadowTransforms.push_back(shadowProj * glm::lookAt(this->pointLightPos, this->pointLightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(this->pointLightPos, this->pointLightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(this->pointLightPos, this->pointLightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(this->pointLightPos, this->pointLightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(this->pointLightPos, this->pointLightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(this->pointLightPos, this->pointLightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    // shader set
    this->shader.setFloat("farPlane", far_plane);
    this->shader.setVec3("light.position", this->pointLightPos);
    this->shader.setVec3("light.diffuse", this->pointLightDiffuse);
    for (unsigned i = 0; i < 6; ++i) {
        this->shader.setMat4("lightShadowMatrices[" + to_string(i) + "]", shadowTransforms[i]);
    }
	/* scene info
    * geom & transform & shader set
    */
    // scene geom
    float quadVertices[] = {
        // pos (normal = color)
        0.f, 0.f, 0.f,  0.0f, -1.0f, 0.0f,
        0.f, 0.f, 5.f,  0.0f, -1.0f, 0.0f,
        5.f, 0.f, 0.f,  0.0f, -1.0f, 0.0f,
        5.f, 0.f, 5.f,  0.0f, -1.0f, 0.0f
    };
    unsigned quadIndices[] = {
        0, 1, 2,
        1, 2, 3
    };
    // quad VAO
    unsigned quadVBO, quadEBO;
    glGenVertexArrays(1, &this->quadVAO);
    glGenBuffers(1, &quadVBO);
    glGenBuffers(1, &quadEBO);
    glBindVertexArray(this->quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    float boxVertices[] = {
    0.f, 0.f, 0.f,  0.0f, 1.0f, 0.0f,
    0.f, 0.f, 1.f,  0.0f, 1.0f, 0.0f,
    1.f, 0.f, 0.f,  0.0f, 1.0f, 0.0f,
    1.f, 0.f, 1.f,  0.0f, 1.0f, 0.0f
    };
    unsigned boxIndices[] = {
    0, 1, 2,
    1, 2, 3
    };
    // box VAO
    unsigned boxVBO, boxEBO;
    glGenVertexArrays(1, &this->boxVAO);
    glGenBuffers(1, &boxVBO);
    glGenBuffers(1, &boxEBO);
    glBindVertexArray(this->boxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, boxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(boxVertices), boxVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boxEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(boxIndices), boxIndices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    
    // scene transform
    glm::mat4 model1(1.f);
    model1 = glm::translate(model1, { 0.f, 5.f, 0.f });
    this->quadModelTransforms.push_back(model1);

    glm::mat4 model2 = glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(1.f, 0.f, 0.f));
    model2 = glm::translate(model2, { 0.f, 0.f, -5.f });
    this->quadModelTransforms.push_back(model2);

    glm::mat4 model3 = glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(-1.f, 0.f, 0.f));
    //model3 = glm::translate(model3, { 0.f, -5.f, 0.f });
    this->quadModelTransforms.push_back(model3);

    glm::mat4 model4 = glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
    model4 = glm::translate(model4, { 0.f, 5.f, -5.f });
    this->quadModelTransforms.push_back(model4);

    glm::mat4 model5(1.f);
    this->quadModelTransforms.push_back(glm::rotate(model5, glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f)));

    glm::vec3 boxTranslate = { 2.f, 1.f, 1.f };

    glm::mat4 boxModel1 = glm::translate(glm::mat4(1.f), boxTranslate);
    this->boxModelTransforms.push_back(boxModel1);

    glm::mat4 boxModel2 = glm::translate(glm::mat4(1.f), boxTranslate);
    boxModel2 = glm::translate(boxModel2, { 0.f, 0.f, 1.f });
    boxModel2 = glm::rotate(boxModel2, glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
    this->boxModelTransforms.push_back(boxModel2);

    glm::mat4 boxModel3 = glm::translate(glm::mat4(1.f), boxTranslate);
    boxModel3 = glm::translate(boxModel3, { 0.f, -1.f, 0.f });
    boxModel3 = glm::rotate(boxModel3, glm::radians(270.f), glm::vec3(1.f, 0.f, 0.f));
    this->boxModelTransforms.push_back(boxModel3);

    glm::mat4 boxModel4 = glm::translate(glm::mat4(1.f), boxTranslate);
    boxModel4 = glm::translate(boxModel4, { 1.f, 0.f, 0.f });
    boxModel4 = glm::rotate(boxModel4, glm::radians(90.f), glm::vec3(0.f, 0.f, -1.f));
    this->boxModelTransforms.push_back(boxModel4);

    glm::mat4 boxModel5 = glm::translate(glm::mat4(1.f), boxTranslate);
    boxModel5 = glm::translate(boxModel5, { 0.f, -1.f, 0.f });
    boxModel5 = glm::rotate(boxModel5, glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
    this->boxModelTransforms.push_back(boxModel5);
}

void GetShadowSamplePass::initLastPass(vector<unsigned> passVAO, vector<unsigned> passTexture)
{
    // do nothing
}
void GetShadowSamplePass::Render()
{
    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
    this->shader.use();

    // !important
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, GetShadowSamplePass::SHADOW_WIDTH, GetShadowSamplePass::SHADOW_HEIGHT);
    glBindVertexArray(this->quadVAO);
    for (unsigned i = 0; i < 5; ++i) {
        glm::mat4 model = this->quadModelTransforms[i];
        this->shader.setMat4("model", model);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(this->boxVAO);
    for (unsigned i = 0; i < 5; ++i) {
        glm::mat4 model = this->boxModelTransforms[i];
        this->shader.setMat4("model", model);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    // !important
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned GetShadowSamplePass::getPassReturn(vector<unsigned>& passVAO, vector<unsigned>& passTexture)
{
    passVAO = vector<unsigned>();
    passVAO.push_back(quadVAO);
    passVAO.push_back(boxVAO);
    passTexture = vector<unsigned>();
    passTexture.push_back(worldPosMap);
    passTexture.push_back(fluxMap);
	return 0;
}

void GetShadowSamplePass::detachPass()
{
    // do nothing
}

void OutputCubeMapPass::initGlobalSettings()
{
    // do nothing
}

void OutputCubeMapPass::initShader()
{
    this->shader = Shader("outputCubeMap.vert", "outputCubeMap.frag");
    this->shader.use();
}

void OutputCubeMapPass::initTexture()
{
    this->shader.setInt("skyboxTexture", 0);
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
    unsigned skyboxVBO;
    glGenVertexArrays(1, &this->skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(this->skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    // NDC
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

void OutputCubeMapPass::initLastPass(vector<unsigned> passVAO, vector<unsigned> passTexture)
{
    const int index = 1;
    if (passTexture.size() > index)
        this->skyboxTexture = passTexture[index];
    else
        cout << "error at OutputCubeMapPass::initLastPass: passTexture.size() < " << index + 1 << endl;
}

void OutputCubeMapPass::Render()
{
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glDepthFunc(GL_LEQUAL);
    this->shader.use();
    
    mat4 view = mat4(mat3(getCamera()->GetViewMatrix())); // remove translation from the view matrix
    mat4 projection = perspective(radians(getCamera()->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    this->shader.setMat4("view", view);
    this->shader.setMat4("projection", projection);

    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(this->skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->skyboxTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    //glDepthFunc(GL_LESS);
}

unsigned OutputCubeMapPass::getPassReturn(vector<unsigned>& passVAO, vector<unsigned>& passTexture)
{
    return 0;
}

void OutputCubeMapPass::detachPass()
{
    // do nothing
}


void Output2DPass::initGlobalSettings()
{
    // do nothing
}

void Output2DPass::initShader()
{
    this->shader = Shader("output2D.vert", "output2D.frag");
    this->shader.use();
}

void Output2DPass::initTexture()
{
    this->shader.setInt("screenTexture", 0);
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
    unsigned screenVBO;
    glGenVertexArrays(1, &this->screenVAO);
    glGenBuffers(1, &screenVBO);
    glBindVertexArray(this->screenVAO);
    glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
    // NDC
    glBufferData(GL_ARRAY_BUFFER, sizeof(screenVertices), &screenVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void Output2DPass::initLastPass(vector<unsigned> passVAO, vector<unsigned> passTexture)
{
    const int index = 0;
    if (passTexture.size() > index)
        this->output2DTexture = passTexture[index];
    else
        cout << "error at Output2DPass::initLastPass: passTexture.size() < " << index + 1 << endl;
}

void Output2DPass::Render()
{
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    this->shader.use();
    glBindVertexArray(screenVAO);
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->output2DTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

unsigned Output2DPass::getPassReturn(vector<unsigned>& passVAO, vector<unsigned>& passTexture)
{
    return 0;
}

void Output2DPass::detachPass()
{
}
