#version 460 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTcoord;

uniform mat4 u_mvp;
uniform mat4 u_world;
uniform float u_time;
uniform vec3 u_color;

out vec3 position;
out vec3 normal;
out vec2 tcoord;
out vec3 color;
out float time;

void main() {
   
   position = (u_world * vec4(aPosition, 1.0)).xyz;
   normal = aNormal;
   tcoord = aTcoord;
   color = u_color;
   time = u_time;
   gl_Position = u_mvp * vec4(aPosition, 1.0);
}
