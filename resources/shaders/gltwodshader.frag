precision highp float;
precision highp usamplerBuffer;

in vec4 colour;
in vec2 texuv;
flat in int paletteid;

out vec4 FragColour;

uniform sampler2D texture0;
uniform usamplerBuffer palbuf;

void main() {
    vec4 fragcol = texture(texture0, texuv);
    int index = int(fragcol.r * 255.0);
    vec4 newcol = vec4(texelFetch(palbuf, int(256 * paletteid + index)));

    if (paletteid > 0)
        if (index > 0)
            fragcol = vec4(newcol.r / 255.0, newcol.g / 255.0, newcol.b / 255.0, 1.0);

    FragColour =  fragcol * colour;
}
