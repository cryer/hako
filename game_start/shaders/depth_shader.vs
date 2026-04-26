#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix; // 光源空间的变换矩阵
uniform mat4 model;

void main()
{
    // 将顶点变换到光源视角下
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}