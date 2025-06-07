#version 330 core

out vec4 FragColor;

in VS_OUT {
	vec3 FragPos;

#ifdef HAS_NORMALS
	vec3 Normal;
#endif

#if defined(HAS_BASE_COLOR_TEX) || defined(HAS_PBR_TEX)
	vec2 TexCoords;
#endif

	vec4 FragPosLightSpace;
} fs_in;

#ifdef HAS_BASE_COLOR_TEX
uniform sampler2D tex;
#else
uniform vec3 base_color;
#endif

uniform sampler2D shadowMap;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform vec3 ambientColor;

// if there's normals, calculate shadows with the bias, if no, without
#ifdef HAS_NORMALS
float ShadowCalculation(vec4 fragPosLightSpace, float bias) {
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get depth of current fragment from light’s perspective
    float currentDepth = projCoords.z;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    int count = 0;
    for(float x = -1; x <= 1; x += .1)
    {
        count++;
        for(float y = -1; y <= 1; y += .1)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= (count * count);

    if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}
#else
float ShadowCalculation(vec4 fragPosLightSpace) {
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get depth of current fragment from light’s perspective
    float currentDepth = projCoords.z;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    int count = 0;
    for(float x = -1; x <= 1; x += .1)
    {
        count++;
        for(float y = -1; y <= 1; y += .1)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= (count * count);

    if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}
#endif

// main func
void main() {
#ifdef HAS_BASE_COLOR_TEX
    vec3 color = texture(tex, fs_in.TexCoords).rgb;
#else
    vec3 color = base_color;
#endif

    // ambient
    vec3 ambient = ambientColor * color;
    
#ifdef HAS_NORMALS
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    vec3 normal = normalize(fs_in.Normal);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = vec3(0.3) * spec; // assuming bright white light color

    // calculate shadow with bias
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace, bias);

#else
    // calculate shadow without bias
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);
#endif

#ifdef HAS_NORMALS
    FragColor = vec4(ambient + (1.0 - shadow) * (diffuse + specular), 1.0);
#else
    FragColor = vec4(ambient + (1.0 - shadow) * color, 1.0);
#endif
}