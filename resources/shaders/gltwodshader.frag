#version 410 core

in vec4 colour;
in vec2 texuv;
flat in uint paletteid;

out vec4 FragColour;

uniform sampler2D texture0;
uniform usamplerBuffer palbuf;
uniform bool repaint;

void main() {
    vec4 fragcol = texture(texture0, texuv);
    uint index = uint(fragcol.r * 255);
    vec4 newcol = texelFetch(palbuf, int( 256 * paletteid + index));

    if (repaint == true) {
	if (paletteid > 0)
	    if (index > 0)
	        fragcol = vec4(newcol.r / 255.0, newcol.g / 255.0, newcol.b / 255.0, 1.0);
   }
	
    FragColour =  fragcol * colour;
} 