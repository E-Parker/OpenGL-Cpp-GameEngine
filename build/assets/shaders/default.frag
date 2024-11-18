#version 460 core

out vec4 FragColor;
in vec3 position;
in vec3 normal;
in vec2 tcoord;

uniform sampler2D albedo;

void main() {

    FragColor = texture(albedo, tcoord);
}

