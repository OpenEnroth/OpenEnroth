#version 410 core

layout (location = 0) in vec3 vaPos;
layout (location = 1) in vec2 vaTexUV;
layout (location = 2) in vec2 vaTexLayer;
layout (location = 3) in vec3 vaNormal;
layout (location = 4) in float vaAttrib;

out vec4 vertexColour;
out vec2 texuv;
flat out vec2 olayer;
out vec3 vsPos;
out vec3 vsNorm;
flat out int vsAttrib;

uniform mat4 view;
uniform mat4 projection;

void main() {
	gl_Position = projection * view * vec4(vaPos, 1.0);

	// rgb unused
	float opacity = smoothstep(1.0 , 0.9999, gl_Position.z / gl_Position.w); 
	vertexColour = vec4(0.0, 0.0, 0.0, opacity);

	texuv = vaTexUV;
	olayer = vaTexLayer;
	vsPos = vaPos;
	vsNorm = vaNormal;
	vsAttrib = int(vaAttrib);
} 