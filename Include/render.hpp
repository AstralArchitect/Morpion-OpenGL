#include <tools/gltfloader.hpp>

#include <iostream>
#include <vector>

namespace Render {
    void renderFrame(GLFWwindow *window, std::vector<GltfModel> & models, glm::mat4 lightSpaceMatrix, GLuint depthMap);
    void renderInputFrame(GLFWwindow *window, GltfModel &plateau, Shader &input_shader);
    void renderDepthFrame(GLFWwindow *window, std::vector<GltfModel> & models, glm::mat4 const& lightSpaceMatrix);
};