#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <tools/camera.hpp>
#include <tools/object.hpp>

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

extern GltfModel horloge;

void Render::renderFrame(GLFWwindow *window, std::vector<GltfModel> &models, glm::mat4 lightSpaceMatrix, GLuint depthMap)
{
    // view/projection/world transformations
    // -------------------------------
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model;

    // uniforms
    // -------
    horloge.set_global_uniforms([&] (Shader* shader) {
        shader->use();
        shader->setVec3("viewPos", camera.Position);
        shader->setVec3("lightPos", lightPos);
        shader->setVec3("ambientColor", backgroundColor);
        shader->setInt("shadowMap", 2);
    }, view, projection);

    // draw
    // ----
    horloge.draw();
}

void Render::renderScene(GLFWwindow *window, std::vector<GltfModel> &models, glm::mat4 const& lightSpaceMatrix)
{
    //
}