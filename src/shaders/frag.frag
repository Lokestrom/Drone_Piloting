#version 460 core

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform GlobalUbo {
    mat4 projection;
 	mat4 view;
    vec4 cameraPos;
    vec4 lightSource; // w for intencity
} ubo;

void main()
{
    vec3 N = normalize(fragNormalWorld);
    vec3 L = normalize(-ubo.lightSource.xyz);

    float NdotL = max(dot(N, L), 0.0);

    vec3 diffuse = fragColor * NdotL * ubo.lightSource.w;
    vec3 ambient = 0.2 * fragColor;

    vec3 finalColor = ambient + diffuse;
    outColor = vec4(finalColor, 1.0);
}