#version 330 core
in vec2 vUV;
uniform sampler2D uFrame;
out vec4 FragColor;
void main() {
    FragColor = vec4(texture(uFrame, vUV).rgb, 1.0);
}
