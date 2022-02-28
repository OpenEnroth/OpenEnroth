#version 410 core

layout (location = 0) in vec3 vaPos;
layout (location = 1) in vec2 vaTexUV;
layout (location = 2) in vec2 vaTexLayer;
layout (location = 3) in vec3 vaNormal;

out vec4 vertexColour;
out vec2 texuv;
flat out vec2 olayer;
out vec3 vsPos;
out vec3 vsNorm;

uniform mat4 view;
uniform mat4 projection;

void main() {
	gl_Position = projection * view * vec4(vaPos, 1.0);

	//unused
	vertexColour = vec4(0.0005 * vaPos.y, 0.30, 0.30, 1.0);

	texuv = vaTexUV;
	olayer = vaTexLayer;
	vsPos = vaPos;
	vsNorm = vaNormal;
}
