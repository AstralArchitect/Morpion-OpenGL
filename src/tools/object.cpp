#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <tools/object.hpp>

#include <iostream>

Object::Object(float *vertices, unsigned long verticesSize, bool UVs, bool normals, bool texCoords, std::string vertexPath, std::string fragmentPath)
{
    createVAO(vertices, verticesSize, UVs, normals, texCoords);
    numVertices = verticesSize / sizeof(float);
    shader = Shader(vertexPath, fragmentPath);
    depthShader = nullptr;
    
    gltf = false;
    depth = false;
}

Object::Object(float *vertices, unsigned long verticesSize, bool UVs, bool normals, bool texCoords, std::string vertexPath, std::string fragmentPath, std::vector<GLuint> new_texts)
{
    createVAO(vertices, verticesSize, UVs, normals, texCoords);
    numVertices = verticesSize / sizeof(float);
    shader = Shader(vertexPath, fragmentPath);
    depthShader = nullptr;
    textures = new_texts;

    gltf = false;
    depth = false;
}

Object::~Object()
{
    if (gltf)
    {
        delete model;

        return;
    }
    if (depth)
    {
        delete depthShader;
    }
    

    textures.clear();
    
    numVertices = 0;
}

void Object::draw()
{
    if (gltf)
    {
        model->draw();
        shader.unuse();
        return;
    }
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, numVertices);
    // unbinde shader & VAO
    glBindVertexArray(0);
    shader.unuse();
}

GLuint Object::getText(unsigned short indice)
{
    return textures[indice];
}

void Object::createVAO(float *vertices, unsigned long verticesSize, bool UVs, bool normals, bool texCoords)
{
    unsigned int VBO;
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, GL_STATIC_DRAW);

    int total_offset = 0;
    total_offset += UVs ? 3 : 0;
    total_offset += normals ? 3 : 0; 
    total_offset += texCoords ? 2 : 0;
    total_offset *= sizeof(float);

    int actual_offset = 0;
    if (UVs)
    {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, total_offset, (void*)actual_offset);
        glEnableVertexAttribArray(0);
        actual_offset += 3;
    }
    if (normals)
    {
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, total_offset, (void*)(actual_offset * sizeof(float)));
        glEnableVertexAttribArray(1);
        actual_offset += 3;
    }
    if (texCoords)
    {
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, total_offset, (void*)(actual_offset * sizeof(float)));
        glEnableVertexAttribArray(2);
        actual_offset += 2;
    }
}