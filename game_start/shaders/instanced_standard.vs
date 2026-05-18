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

void main()
{
    TexCoords = aTexCoords;
    vec4 worldPos = instanceModel * vec4(aPos, 1.0);
    gl_Position = projection * view * worldPos;
    Normal = mat3(transpose(inverse(instanceModel))) * aNormal;
    FragPos = worldPos.xyz;
    FragPosLightSpace = lightSpaceMatrix * worldPos;
}
