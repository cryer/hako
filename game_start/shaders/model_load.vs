#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;



out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;  // 世界坐标位置(左乘model)，用于光照计算
// 阴影计算
out vec4 FragPosLightSpace;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// 阴影计算
uniform mat4 lightSpaceMatrix;

void main()
{
    TexCoords = aTexCoords;    
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    Normal = mat3(transpose(inverse(model))) * aNormal;
    FragPos = vec3(model * vec4(aPos, 1.0));

    // 计算当前顶点在光源空间的位置
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
}