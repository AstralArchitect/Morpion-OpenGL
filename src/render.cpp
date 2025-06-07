#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <tools/camera.hpp>

#include "render.hpp"

// settings
extern unsigned int SCR_WIDTH;
extern unsigned int SCR_HEIGHT;

// camera
extern Camera camera;

// timing
extern float deltaTime;
extern float lastFrame;

extern glm::vec3 lightPos;
extern glm::vec3 pointLightColor;

// background strength
extern glm::vec3 backgroundColor;

void Render::renderFrame(GLFWwindow *window, std::vector<GltfModel> &models, glm::mat4 lightSpaceMatrix, GLuint depthMap)
{
    // view/projection/world transformations
    // -------------------------------
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model;

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    // uniforms
    // -------
    models[0].set_global_uniforms([&] (Shader* shader) {
        shader->use();
        shader->setVec3("viewPos", camera.Position);
        shader->setVec3("lightPos", lightPos);
        shader->setVec3("ambientColor", backgroundColor);
        shader->setInt("shadowMap", 4);
    }, view, projection);

    // draw
    // ----
    for (auto &model : models)
        model.draw();
}

void Render::renderInputFrame(GLFWwindow *window, GltfModel &plateau, Shader &input_shader) {
    // view/projection/world transformations
    // -------------------------------
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(0.0, sin(glfwGetTime()), 0.0));
    
    input_shader.use();
    input_shader.setMat4("transform", projection * view * model);

    plateau.draw(true);
}

void Render::renderDepthFrame(GLFWwindow *window, std::vector<GltfModel> &models, glm::mat4 const& lightSpaceMatrix)
{
    for(auto &model : models)
        model.draw(lightSpaceMatrix);
}