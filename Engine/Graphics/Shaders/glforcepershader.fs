#version 410 core

in vec4 colour;
in vec4 texuv;

out vec4 FragColour;

uniform sampler2D texture0;

void main() {
    FragColour = textureProj(texture0, texuv) * colour;
} 