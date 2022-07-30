#version 410 core

in vec4 vertexColour;
in vec2 texuv;
//flat in float olayer;
//in vec3 vsPos;
//in vec3 vsNorm;
//flat in int vsAttrib;

out vec4 FragColour;


uniform sampler2D texture0;



void main() {

	vec4 fragcol = texture(texture0, texuv);
	    
	FragColour = vertexColour * fragcol;

}
