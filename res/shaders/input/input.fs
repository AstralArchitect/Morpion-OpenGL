#version 330 core

in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D text;

void main()
{
    FragColor = texture(text, TexCoords);
}
