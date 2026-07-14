#include "precision.glsl"

in mat4 colours;
in vec2 texuv;

out vec4 FragColour;

uniform sampler2D texture0;

void main() {
    // Texture channels hold per-channel color weights summing to 1, so this multiplication blends the four
    // per-channel colors passed in the columns of the colours matrix.
    FragColour = colours * texture(texture0, texuv);
}
