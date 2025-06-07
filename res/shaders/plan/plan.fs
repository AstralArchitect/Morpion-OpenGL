#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D colorMap;
uniform sampler2D specularMap;
uniform sampler2D shadowMap;

uniform vec3 viewPos;

layout (std140) uniform Light {
                            // base alignement      // aligned offset
    vec3  lightColor;       // 16                   // 0
    float lightConstant;    // 4                    // 16
    float lightLinear;      // 4                    // 20
    float lightQuadratic;   // 4                    // 24
};
uniform vec3 lightPos;

uniform vec3 ambientColor;

float ShadowCalculation(vec4 fragPosLightSpace, float bias)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get depth of current fragment from light’s perspective
    float currentDepth = projCoords.z;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    int count = 0;
    for(float x = -1; x <= 1; x += .5)
    {
        count++;
        for(float y = -1; y <= 1; y += .5)
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

void main() {
    vec3 color = texture(colorMap, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    // ambient
    vec3 ambient = (color * ambientColor);
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 256);
    vec3 specular = spec * lightColor * texture(specularMap, fs_in.TexCoords).rgb;
    // calculate shadow
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace, bias);
    // attenuation
    float distance = length(lightPos - fs_in.FragPos);
    float attenuation = 1.0 / (lightConstant + lightLinear * distance + lightQuadratic * distance);
    diffuse *= attenuation;
    specular *= attenuation;

    vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

    result = pow(result, vec3(1.0/2.2)); // gamma correction

    FragColor = vec4(result, 1.0);
}