layout (location = 0) in vec3 vaPos;
layout (location = 1) in vec3 vaCol;

out vec3 colour;

uniform mat4 view;
uniform mat4 projection;

void main() {
    vec4 adjpos = view * vec4(vaPos, 1.0);
    adjpos.x += 0.5;
    adjpos.y += 0.5;
    gl_Position = projection * adjpos; 
    colour = vaCol;
}
