#version 460 core

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 uv;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform GlobalUbo {
    mat4 projection;
 	mat4 view;
    vec4 cameraPos;
    vec4 lightSource; // w for intencity
} ubo;

layout(set = 1, binding = 0) uniform sampler2D tex;

void main()
{
    vec3 N = normalize(fragNormalWorld);
    vec3 L = normalize(-ubo.lightSource.xyz);

    float NdotL = max(dot(N, L), 0.0);

    vec3 diffuse = fragColor * NdotL * ubo.lightSource.w;
    vec3 ambient = 0.5 * fragColor;

    vec3 lighting = ambient + diffuse;

    vec3 albedo = texture(tex, uv).rgb;

    outColor = vec4(lighting * albedo, 1.0);
}