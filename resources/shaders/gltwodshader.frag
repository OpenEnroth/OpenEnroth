#version 410 core

in vec4 colour;
in vec2 texuv;

out vec4 FragColour;

uniform sampler2D texture0;

void main() {
    FragColour = texture(texture0, texuv) * colour;
} 