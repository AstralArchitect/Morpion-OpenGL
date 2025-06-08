#include <tools/gltfloader.hpp>

#include <iostream>
#include <vector>

namespace Render {
    void renderFrame(GLFWwindow *window, glm::mat4 lightSpaceMatrix, GLuint depthMap);
    void renderDepthFrame(GLFWwindow *window, glm::mat4 const& lightSpaceMatrix);
};