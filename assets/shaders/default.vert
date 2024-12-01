#version 460 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTcoord;

uniform mat4 u_mvp;
uniform float u_time;

out vec3 position;
out vec3 normal;
out vec2 tcoord;
out float time;

void main() {
    
    position = aPosition;
    normal = aNormal;
    tcoord = aTcoord;
    time = u_time;

    gl_Position = u_mvp * vec4(aPosition, 1.0);
}
