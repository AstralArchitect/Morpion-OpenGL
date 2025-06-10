#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <tools/camera.hpp>

#include "render.hpp"

#include <mutex>

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

extern std::mutex mtx;
extern std::vector<int> winIndexes;

extern bool won_flag;

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
    mtx.lock();
    for (auto &model : renderList)
        model.set_global_uniforms([&] (Shader* shader) {
            shader->use();
            shader->setVec3("viewPos", camera.Position);
            shader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
        }, view, projection);
    mtx.unlock();
    // draw
    // ----
    mtx.lock();
    for (int i = 0; i < renderList.size(); ++i) {
        auto &model = renderList[i];

        // Determine if the current model's index 'i' is in the list of winning indexes
        bool is_winning_piece = false;
        for (int winning_index : winIndexes) {
            if (winning_index == i) {
                is_winning_piece = true;
                break; // Found a match, no need to search further
            }
        }

        if (is_winning_piece) model.set_global_uniforms([&] (Shader* shader) {
                shader->use();
                shader->setVec3("override_color", glm::vec3(.0f, 1.f, .0f));
                shader->setInt("override", 1);
            });
        else model.set_global_uniforms([&] (Shader* shader) {
                shader->use();
                shader->setInt("override", 0);
            });

        model.draw();
    }
    mtx.unlock();
}

void Render::renderDepthFrame(GLFWwindow *window, glm::mat4 const& lightSpaceMatrix)
{
    mtx.lock();
    for(auto &model : renderList)
        model.draw(lightSpaceMatrix);
    mtx.unlock();
}