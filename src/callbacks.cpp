#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <tools/camera.hpp>
#include <tools/gltfloader.hpp>
#include <callbacks.hpp>

#include <stb_image.h>
#include <iostream>
#include <thread>
#include <mutex>

extern Camera camera;

extern float deltaTime;

extern unsigned int SCR_WIDTH;
extern unsigned int SCR_HEIGHT;

// model, view and projection matrix
extern glm::mat4 projection;
extern glm::mat4 view;
extern glm::mat4 model;

double lastX = SCR_WIDTH / 2.0;
double lastY = SCR_HEIGHT / 2.0;
bool firstMouse = true;

// render and models lists
std::vector<int> types = {0};
extern std::vector<GltfModel> renderList;
extern std::vector<GltfModel> loadedModels;

std::mutex mtx; // mutex for critical section

bool won_flag = false;
std::vector<int> winIndexes;

std::pair<int, int> positionsMatrix[3][3] = 
{
    {{0, 0}, {0, 0}, {0, 0}},
    {{0, 0}, {0, 0}, {0, 0}},
    {{0, 0}, {0, 0}, {0, 0}}
};

#ifdef _WIN32
#include <windows.h>

void sleep_ms(DWORD milliseconds) {
    Sleep(milliseconds);
}

#else

#include <time.h>

void sleep_ms(unsigned long milliseconds) {
    struct timespec ts;

    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;

    nanosleep(&ts, NULL);
}


#endif

void Callback::Shadow_info::init()
{
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void Callback::processInput(GLFWwindow *window)
{
    static bool isFullscreen = false;
    static int windowedWidth, windowedHeight, windowedPosX, windowedPosY;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(LEFT, deltaTime);
        camera.ProcessMouseMovement(deltaTime * 575, 0);
    }
    else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(RIGHT, deltaTime);
        camera.ProcessMouseMovement(-(deltaTime) * 575, 0);
    }
    
    if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS) {
        if (!isFullscreen) {
            glfwGetWindowSize(window, &windowedWidth, &windowedHeight);
            glfwGetWindowPos(window, &windowedPosX, &windowedPosY);

            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
        } else {
            glfwSetWindowMonitor(window, NULL, windowedPosX, windowedPosY, windowedWidth, windowedHeight, 0);
        }
        isFullscreen = !isFullscreen;
        sleep_ms(100);
    }
    
}

void Callback::mouse(GLFWwindow * window, double xposIn, double yposIn) {
    if (firstMouse)
    {
        lastX = xposIn;
        lastY = yposIn;
        firstMouse = false;
    }

    float xoffset = xposIn - lastX;
    float yoffset = lastY - yposIn; // reversed since y-coordinates go from bottom to top
    lastX = xposIn;
    lastY = yposIn;

    camera.ProcessMouseMovement(xoffset, yoffset, 1);
}

void won(std::vector<std::pair<int, int>> pos, int winner)
{
    std::vector<int> lWinIndexes;
    for(auto &i : pos) {
        lWinIndexes.push_back(positionsMatrix[i.first][i.second].second);
    }
    winIndexes = lWinIndexes;
    won_flag = true;
    sleep_ms(1000);
    mtx.lock();
    renderList.clear();
    renderList.push_back(loadedModels[0]);
    types.clear();
    types.push_back(0);
    memset(positionsMatrix, 0, sizeof(positionsMatrix));
    mtx.unlock();
    won_flag = false;
    winIndexes.clear();
}

void Callback::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    // Nous voulons seulement réagir à un clic gauche (bouton 0)
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        glfwGetCursorPos(window, &lastX, &lastY); // Obtenir la position actuelle du curseur

        float screenX = static_cast<float>(lastX);
        float screenY = static_cast<float>(SCR_HEIGHT - lastY - 1); // Inverser l'axe Y

        glm::vec3 ndc_near(
            (screenX / (float)SCR_WIDTH) * 2.0f - 1.0f,
            (screenY / (float)SCR_HEIGHT) * 2.0f - 1.0f,
            -1.0f // Z pour le plan proche
        );

        glm::vec3 ndc_far(
            (screenX / (float)SCR_WIDTH) * 2.0f - 1.0f,
            (screenY / (float)SCR_HEIGHT) * 2.0f - 1.0f,
            1.0f // Z pour le plan lointain
        );

        glm::mat4 inverseProjView = glm::inverse(projection * view);

        glm::vec4 ray_clip_near = glm::vec4(ndc_near.x, ndc_near.y, ndc_near.z, 1.0f);
        glm::vec4 ray_world_near = inverseProjView * ray_clip_near;
        ray_world_near /= ray_world_near.w;

        glm::vec4 ray_clip_far = glm::vec4(ndc_far.x, ndc_far.y, ndc_far.z, 1.0f);
        glm::vec4 ray_world_far = inverseProjView * ray_clip_far;
        ray_world_far /= ray_world_far.w;

        glm::vec3 ray_origin = glm::vec3(ray_world_near);
        glm::vec3 ray_direction = glm::normalize(glm::vec3(ray_world_far) - ray_origin);

        glm::vec3 plane_point = glm::vec3(0.0f, -1.0f, 0.0f);
        glm::vec3 plane_normal = glm::vec3(0.0f, 1.0f, 0.0f);

        float denominator = glm::dot(ray_direction, plane_normal);

        if (glm::abs(denominator) < 0.0001f) {
            std::cout << "Le rayon est parallèle au plan ou ne l'intersecte pas." << std::endl;
        } else {
            float t = glm::dot(plane_point - ray_origin, plane_normal) / denominator;
            
            if (t >= 0.0f) {
                glm::vec3 intersection_point = ray_origin + t * ray_direction;
                    
                // create a copy of the model
                GltfModel model = renderList.size() % 2 == 0 ? loadedModels[1] : loadedModels[2];
                    
                int posx_mat = (float)((float)min((int)(abs(intersection_point.x) * 3), 1) * sign(intersection_point.x)) + 1;
                int posy_mat = (float)((float)min((int)(abs(intersection_point.z) * 3), 1) * sign(intersection_point.z)) + 1;
                
                if(abs(intersection_point.x) > 1 || abs(intersection_point.z) > 1 || positionsMatrix[posx_mat][posy_mat].first == 1 || positionsMatrix[posx_mat][posy_mat].first == 2) return;
                else if (positionsMatrix[posx_mat][posy_mat].first == 0) {
                    positionsMatrix[posx_mat][posy_mat].first = renderList.size() % 2 == 0 ? 1 : 2;
                    positionsMatrix[posx_mat][posy_mat].second = renderList.size();
                }

                float posx_space = (posx_mat - 1.0f) * 2/3;
                float posy_space = (posy_mat - 1.0f) * 2/3;

                model.set_global_uniforms(glm::scale(glm::translate(glm::mat4(1.f), glm::vec3(posx_space, -1.0, posy_space)), glm::vec3(0.25)));
                renderList.push_back(model);
                types.push_back(renderList.size() % 2 == 0 ? 1 : 2);

                // check if a line of 3 is reached
                for (int i = 0; i < 3; ++i) {
                    // Check rows
                    if (positionsMatrix[i][0].first != 0 && positionsMatrix[i][0].first == positionsMatrix[i][1].first && positionsMatrix[i][1].first == positionsMatrix[i][2].first) {
                        std::cout << "Player " << positionsMatrix[i][0].first << " wins!" << std::endl;
                        std::vector<std::pair<int, int>> pos = {{i, 0}, {i, 1}, {i, 2}};
                        try {
                            std::thread t(won, pos, positionsMatrix[i][0].first);
                            t.detach();
                        } catch (std::exception& e) {
                            std::cout << "Exception caught: " << e.what() << std::endl;
                        }
                    }
                    // Check columns
                    if (positionsMatrix[0][i].first != 0 && positionsMatrix[0][i].first == positionsMatrix[1][i].first && positionsMatrix[1][i].first == positionsMatrix[2][i].first) {
                        std::cout << "Player " << positionsMatrix[0][i].first << " wins!" << std::endl;
                        std::vector<std::pair<int, int>> pos = {{0, i}, {1, i}, {2, i}};
                        try {
                            std::thread t(won, pos, positionsMatrix[0][i].first);
                            t.detach();
                        } catch (std::exception& e) {
                            std::cout << "Exception caught: " << e.what() << std::endl;
                        }
                    }
                }
            
                // Check diagonals
                if (positionsMatrix[0][0].first != 0 && positionsMatrix[0][0].first == positionsMatrix[1][1].first && positionsMatrix[1][1].first == positionsMatrix[2][2].first) {
                    std::cout << "Player " << positionsMatrix[0][0].first << " wins!" << std::endl;
                    std::vector<std::pair<int, int>> pos = {{0, 0}, {1, 1}, {2, 2}};
                    try {
                        std::thread t(won, pos, positionsMatrix[0][0].first);
                        t.detach();
                    } catch (std::exception& e) {
                        std::cout << "Exception caught: " << e.what() << std::endl;
                    }
                }
                if (positionsMatrix[0][2].first != 0 && positionsMatrix[0][2].first == positionsMatrix[1][1].first && positionsMatrix[1][1].first == positionsMatrix[2][0].first) {
                    std::cout << "Player " << positionsMatrix[0][2].first << " wins!" << std::endl;
                    std::vector<std::pair<int, int>> pos = {{0, 2}, {1, 1}, {2, 0}};
                    try {
                        std::thread t(won, pos, positionsMatrix[0][2].first);
                        t.detach();
                    } catch (std::exception& e) {
                        std::cout << "Exception caught: " << e.what() << std::endl;
                    }
                }
            } else {
                std::cout << "L'intersection est derrière la caméra." << std::endl;
            }
        }
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void Callback::framebuffer_size(GLFWwindow* window, int width, int height)
{
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void Callback::scroll(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

GLFWwindow *createContextAndWindows(const unsigned int SCR_WIDTH, const unsigned int SCR_HEIGHT, char *WindowTitle, char *error)
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 128);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, WindowTitle, NULL, NULL);
    if (window == NULL)
    {
        glfwTerminate();
        *error = (char)1;
        return NULL;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, Callback::framebuffer_size);
    glfwSetScrollCallback(window, Callback::scroll);
    //glfwSetCursorPosCallback(window, Callback::mouse);
    glfwSetMouseButtonCallback(window, Callback::mouse_button_callback);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        *error = (char)2;
        return NULL;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return window;
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const *path, bool gammaCorrection, GLuint mode)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum internalFormat;
        GLenum dataFormat;
        if (nrComponents == 1)
        {
            internalFormat = dataFormat = GL_RED;
        }
        else if (nrComponents == 3)
        {
            internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrComponents == 4)
        {
            internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}