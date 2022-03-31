#include "shAlgorithm.h"
int main() {

    GLFWwindow* window = initWindow();

    // --------shader--------
    // ����гϵ����Ⱦ����
    //Shader sHshader("sh.vert", "sh.frag");

    // --------texture--------
    // ��պ�����
    stbi_set_flip_vertically_on_load(false);
    vector<string> paths = {
        string("../assets/skybox/skybox0.png"),
        string("../assets/skybox/skybox1.png"),
        string("../assets/skybox/skybox2.png"),
        string("../assets/skybox/skybox3.png"),
        string("../assets/skybox/skybox4.png"),
        string("../assets/skybox/skybox5.png"),
    };
    vector<unsigned char*> datas;
    int textureSize = 0;
    int nrComponents;
    unsigned skyboxCubemap = loadCubemap(paths, textureSize, nrComponents, datas);

    vector<SHSample> samples;
    SH_setup_spherical_samples(samples, 50);

    // ��������������гϵ��
    set_sh_light_from_cube_paraments(datas, textureSize, nrComponents);
    vector<glm::vec3> sh_light = SH_project_vector_function_rgb(gen_sh_light_from_cube, samples);
    for (const glm::vec3& sh : sh_light) {
        cout << sh.x << ", " << sh.y << ", " << sh.z << endl;
    }


    // --------������гϵ��--------


    // --------rendering settings--------

    // --------��Ⱦѭ��--------
    //while (!glfwWindowShouldClose(window)) {
    //    updateDeltaTime(float(glfwGetTime()));
    //    processInput(window);
    //    glClearColor(0.1f, 0.1f, 0.1f, 1.f);
    //    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //    glEnable(GL_DEPTH_TEST);
    //    glDepthFunc(GL_LEQUAL);
    //    sHshader.use();
    //    // ȥ�������λ�����ԣ�������Ⱦ����������պУ�������������
    //    mat4 view = mat4(mat3(camera.GetViewMatrix()));
    //    sHshader.setMat4("view", view);
    //    mat4 proj = perspective(radians(camera.Zoom), float(SCR_WIDTH / SCR_HEIGHT), 0.1f, 100.f);
    //    sHshader.setMat4("projection", proj);
    //    renderCube();
    //    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    //    glEnable(GL_DEPTH_TEST);
    //    glDepthFunc(GL_LEQUAL);

    //    glfwSwapBuffers(window);
    //    glfwPollEvents();

    //}

    
}