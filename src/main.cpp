#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <tools/camera.hpp>
#include <tools/shader.hpp>
#include <tools/gltfloader.hpp>

#include "OpenGLTools/savstb_image.h"

#include "callbacks.hpp"
#include "render.hpp"

#include <cstdio>

// settings
unsigned int SCR_WIDTH  = 800;
unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 2.5f));

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting info
// -------------
glm::vec3 lightPos(1.0f, 1.0, 0.0f);
glm::vec3 pointLightColor = glm::vec3(1.0f, 0.9f, 0.8f);

// background strength
glm::vec3 backgroundColor(.5f);

// global render list and loaded models
std::vector<GltfModel> renderList;
std::vector<GltfModel> loadedModels;

int main()
{
    // glfw window creation
    // --------------------
    char error;
    GLFWwindow* window = createContextAndWindows(SCR_WIDTH, SCR_HEIGHT, (char*)"Morpion", &error);
    if (window == NULL)
    {
        if (error == 1)
        {
            printf("Failed to create GLFW window");
        }
        else if (error == 2)
        {
            printf("Failed to initialize GLAD");
        }
        return -1;
    }

    // shadows
    // -------
    Callback::Shadow_info shadow_info = {2048, 2048, 0, 0}; 
    shadow_info.init();

    // shaders
    // -------
    ShaderStore shader_store("./res/shaders/pbr");

    // models
    // ------
    // plateau
    GltfModel plateau("./res/models/plateau.glb", shader_store);    
    plateau.set_global_uniforms(glm::translate(glm::mat4(1.f), glm::vec3(0.0, -1, 0.0)));
    loadedModels.push_back(plateau);
    // croix
    GltfModel croix("./res/models/croix.glb", shader_store);
    loadedModels.push_back(croix);
    // rond
    GltfModel rond("./res/models/rond.glb", shader_store);
    loadedModels.push_back(rond);
    // uniforms
    for (auto &model : loadedModels)
        model.set_global_uniforms([&] (Shader* shader) {
            shader->use();
            shader->setVec3("lightPos", lightPos);
            shader->setVec3("ambientColor", backgroundColor);
            shader->setInt("shadowMap", 4);
        });

    // add plateau to render list render list
    renderList.push_back(plateau);
    
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
        Callback::processInput(window);

        // render
        // ------
        glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 0.1f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 1. render depth of scene to texture (from light's perspective)
        // --------------------------------------------------------------
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = .1f, far_plane = 7.5f;
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;

        glViewport(0, 0, shadow_info.SHADOW_WIDTH, shadow_info.SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, shadow_info.depthMapFBO);
            glClear(GL_DEPTH_BUFFER_BIT);
            glCullFace(GL_FRONT);
            Render::renderDepthFrame(window, lightSpaceMatrix);
            glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // reset viewport
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 2. render scene as normal using the generated depth/shadow map  
        // --------------------------------------------------------------
        Render::renderFrame(window, lightSpaceMatrix, shadow_info.depthMap);

        GLenum err = 1;
        while (err != 0) {
            if (err != 0 && err != 1) {
                std::cout << "OpenGL Error occured: " << err << std::endl;
            }

            err = glGetError();
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}