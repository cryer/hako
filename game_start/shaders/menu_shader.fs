#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 color;
uniform int useTexture;
uniform sampler2D image;

void main() {
    if(useTexture == 1) {
        vec4 texColor = texture(image, TexCoords);
        FragColor = texColor * vec4(color, 1.0);
    } else {
        FragColor = vec4(color, 0.8); // 纯色带一点透明度
    }
}