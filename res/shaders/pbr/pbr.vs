#version 330 core

layout (location = 0) in vec3 aPos;

#if defined(HAS_BASE_COLOR_TEX) || defined(HAS_PBR_TEX)
layout (location = 1) in vec2 aTexCoords;
#endif

#ifdef HAS_NORMALS
layout (location = 2) in vec3 aNormal;
#endif

uniform mat4 transform;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;

out VS_OUT {
	vec3 FragPos;

#ifdef HAS_NORMALS
	vec3 Normal;
#endif

#if defined(HAS_BASE_COLOR_TEX) || defined(HAS_PBR_TEX)
	vec2 TexCoords;
#endif

	vec4 FragPosLightSpace;
} vs_out;

void main()
{
	vs_out.FragPos = vec3(model * vec4(aPos, 1.0));

#ifdef HAS_NORMALS
	vs_out.Normal = mat3(transpose(inverse(model))) * aNormal;
#endif

#if defined(HAS_BASE_COLOR_TEX) || defined(HAS_PBR_TEX)
	vs_out.TexCoords = aTexCoords;
#endif

	vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
	
	gl_Position = transform * vec4(aPos, 1.0);
}