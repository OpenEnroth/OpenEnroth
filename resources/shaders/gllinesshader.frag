#ifdef GL_ES
    precision highp float;
#endif

in vec4 colour;

out vec4 FragColour;

void main() {
    FragColour = colour;
}
