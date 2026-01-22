#version 450 core
out vec4 FragColor;
uniform vec3 uColor; // 由程序传入颜色

void main() {
    FragColor = vec4(uColor, 1.0);
}