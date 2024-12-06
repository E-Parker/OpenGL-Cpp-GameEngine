#version 460 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTcoord;


layout (std140) uniform FrameData {
    mat4 u_camera;
    float u_time;
};

uniform mat4 u_mvp;

out vec3 position;
out vec3 normal;    
out vec2 tcoord;
out float time;

void main() {
    
    position = (u_mvp * vec4(aPosition, 1.0)).xyz;

    normal = aNormal;
    tcoord = aTcoord;
    time = u_time;
    gl_Position = u_camera * u_mvp * vec4(aPosition, 1.0);
}
