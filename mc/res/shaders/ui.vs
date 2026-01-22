#version 450 core
layout (location = 0) in vec2 aPos;
// 我们不再依赖顶点颜色，而是用 uniform 控制图标颜色
uniform mat4 model;

void main() {
    // UI 坐标直接对应屏幕空间 (-1 ~ 1)
    gl_Position = model * vec4(aPos, 0.0, 1.0); 
}