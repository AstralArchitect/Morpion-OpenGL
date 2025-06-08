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

// model, view and projection matrix
glm::mat4 projection;
glm::mat4 view;
glm::mat4 model;

// render list
extern std::vector<GltfModel> renderList;

void Render::renderFrame(GLFWwindow *window, glm::mat4 lightSpaceMatrix, GLuint depthMap)
{
    // view/projection/world transformations
    // -------------------------------
    projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    view = camera.GetViewMatrix();

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    // uniforms
    // -------
    for (auto &model : renderList)
    {
        model.set_global_uniforms([&] (Shader* shader) {
            shader->use();
            
            shader->setVec3("viewPos", camera.Position);
            shader->setVec3("lightPos", lightPos);
            shader->setVec3("ambientColor", backgroundColor);

            shader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
            shader->setInt("shadowMap", 4);
        }, view, projection);
    }
    
    // draw
    // ----
    for (auto &model : renderList)
        model.draw();
}

void Render::renderDepthFrame(GLFWwindow *window, glm::mat4 const& lightSpaceMatrix)
{
    for(auto &model : renderList)
        model.draw(lightSpaceMatrix);
}