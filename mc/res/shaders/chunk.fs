#version 450 core
out vec4 FragColor;

in vec3 Color;
in vec3 Normal;

void main() {
    // 简单的光照
    vec3 lightDir = normalize(vec3(0.4, 0.8, 0.5));
    float diff = max(dot(Normal, lightDir), 0.25); // 0.25 是环境光强度
    FragColor = vec4(Color * diff, 1.0);
}