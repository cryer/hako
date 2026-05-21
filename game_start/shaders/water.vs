#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec2 TexCoords;
out vec4 FragPosLightSpace;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 lightSpaceMatrix;
uniform float time;

void main()
{
    vec3 pos = aPos;
    float wave1 = sin(pos.x * 1.8 + time * 1.5) * 0.10;
    float wave2 = sin(pos.z * 2.1 + time * 1.2) * 0.08;
    float wave3 = sin((pos.x + pos.z) * 1.5 + time * 2.0) * 0.05;
    pos.y += wave1 + wave2 + wave3;

    FragPos = pos;
    TexCoords = aTexCoords;
    gl_Position = projection * view * vec4(pos, 1.0);
    FragPosLightSpace = lightSpaceMatrix * vec4(pos, 1.0);
}
