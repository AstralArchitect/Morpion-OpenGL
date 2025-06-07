#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 transform;
uniform mat4 model;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoord;
    gl_Position = transform * vec4(aPos, 1.0);
}