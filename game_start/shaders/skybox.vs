#version 330 core
layout (location = 0) in vec3 aPos;

// 立方体贴图不要纹理坐标，因为顶点位置就是纹理坐标

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    // 天空盒着色器的z分量设置成w，这样透视除法后就始终为1，保证不会遮盖场景中的物体
    // 另外需要注意设置深度函数，将它从默认的GL_LESS改为GL_LEQUAL，小于等于1都在天空盒前面
    gl_Position = pos.xyww; 
}