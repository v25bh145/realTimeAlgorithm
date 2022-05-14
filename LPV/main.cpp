#include "renderDefault.h"
#include "renderAlgorithm.h"

using namespace std;
using namespace glm;


int main() {
	GLFWwindow* window = initWindow();

    RenderAlgorithm ra = RenderAlgorithm();
    
    ra.globalSettings();

    ra.prepareRendering();

    while (!glfwWindowShouldClose(window)) {
        updateDeltaTime(float(glfwGetTime()));
        processInput(window);

        ra.renderingOnce();

        glfwSwapBuffers(window);
        glfwPollEvents();

    }
}