#ifndef OBJECT_H
#define OBJECT_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <tools/shader.hpp>
#include <tools/gltfloader.hpp>

class Object
{
private:
    // VAO / model
    GLuint VAO;
    GltfModel *model;
    
    unsigned int numVertices;
    std::vector<GLuint> textures;
    // mode
    bool gltf;
    bool depth;

    // create VAO from vertices, verticesSize must be the size in byte
    void createVAO(float *vertices, unsigned long verticesSize, bool UVs, bool normals, bool texCoords);
public:
    // shader
    Shader shader;
    Shader *depthShader;

    // constructors
    Object(std::string modelPath, std::string vertexPath, std::string fragmentPath, std::string depthVertexPath, std::string depthFragmentPath);
    Object(std::string modelPath, std::string vertexPath, std::string fragmentPath);
    Object(float *vertices, unsigned long verticesSizeInByte, bool UVs, bool normals, bool texCoords, std::string vertexPath, std::string fragmentPath);
    Object(float *vertices, unsigned long verticesSizeInByte, bool UVs, bool normals, bool texCoords, std::string vertexPath, std::string fragmentPath, std::vector<GLuint> new_text);
    // destructor
    ~Object();

    // shader's world properties func
    void setWorld(glm::mat4 projection, glm::mat4 view);

    // text func
    GLuint getText(unsigned short indice);

    // draw funcs
    void draw();
    void drawWithoutTexture();
};

#endif