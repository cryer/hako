#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

layout (location = 7) in mat4 instanceModel;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosLightSpace;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

uniform float time;
uniform vec3  windDirection;
uniform float windStrength;
uniform float windSpeed;

void main()
{
    vec4 worldPos = instanceModel * vec4(aPos, 1.0);

    // Wind sway: upper vertices sway more, base stays rooted
    float heightFactor = max(aPos.y, 0.0);
    vec2 windDirXZ = normalize(windDirection.xz);
    float posAlongWind = dot(worldPos.xz, windDirXZ);

    float wave  = sin(time * windSpeed + posAlongWind * 2.5) * windStrength * heightFactor;
    wave += sin(time * windSpeed * 1.3 + posAlongWind * 5.0 + 1.5) * windStrength * 0.35 * heightFactor;
    wave += sin(time * windSpeed * 0.7 + posAlongWind * 1.2 + 3.0) * windStrength * 0.2 * heightFactor;

    worldPos.x += windDirXZ.x * wave;
    worldPos.z += windDirXZ.y * wave;

    TexCoords = aTexCoords;
    gl_Position = projection * view * worldPos;
    Normal = mat3(transpose(inverse(instanceModel))) * aNormal;
    FragPos = worldPos.xyz;
    FragPosLightSpace = lightSpaceMatrix * worldPos;
}
