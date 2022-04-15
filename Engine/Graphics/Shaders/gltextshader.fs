#version 410 core

in vec4 colour;
in vec2 texuv;
flat in float layer;

out vec4 FragColour;

uniform sampler2D texture0;
uniform sampler2D texture1;

void main() {
    vec4 col;
    if (layer == 0) {
        col = texture(texture0, texuv) * colour;
    } else {
        col = texture(texture1, texuv) * colour;
    }

    FragColour = col;
}