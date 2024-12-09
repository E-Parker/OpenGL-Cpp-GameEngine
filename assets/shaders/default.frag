#version 460 core

struct Light {
  vec3 position;        // 16   0
  vec3 direction;       // 16   16
  vec3 color;           // 16   32
  float attenuation;    // 4    48
};

out vec4 FragColor;
in vec3 position;
in vec3 normal;
in vec2 tcoord;
in int activeLights;
//in Light[128] lights;

uniform sampler2D albedo;

vec3 phong(vec3 position, vec3 normal, vec3 camera, vec3 light, vec3 color, float ambientFactor, float diffuseFactor, float specularPower) {
    vec3 N = normalize(normal);
    vec3 L = normalize(light - position);
    vec3 V = normalize(camera - position);
    vec3 R = normalize(reflect(-L, N));
    float dotNL = max(dot(N, L), 0.0);
    float dotVR = max(dot(V, R), 0.0);

    vec3 lighting = vec3(0.0);
    vec3 ambient = color * ambientFactor;
    vec3 diffuse = color * dotNL * diffuseFactor;
    vec3 specular = color * pow(dotVR, specularPower);

    lighting += ambient;
    lighting += diffuse;
    lighting += specular;
    return lighting;
}

vec3 point_light(vec3 position, vec3 normal, vec3 camera, vec3 light, vec3 color, float ambientFactor, float diffuseFactor, float specularPower, float radius) {
    vec3 lighting = phong(position, normal, camera, light, color, ambientFactor, diffuseFactor, specularPower);

    float dist = length(light - position);
    float attenuation = clamp(radius / dist, 0.0, 1.0);
    lighting *= attenuation;

    return lighting;
}

vec3 direction_light(vec3 direction, vec3 normal, vec3 camera, vec3 color, float ambientFactor, float diffuseFactor, float specularPower) {
    vec3 lighting = phong(vec3(0.0), normal, camera, -direction, color, ambientFactor, diffuseFactor, specularPower);
    return lighting;
}

vec3 spot_light(vec3 position, vec3 direction, vec3 normal, vec3 camera, vec3 light, vec3 color, float ambientFactor, float diffuseFactor, float specularPower, float radius, float fov) {
    
    vec3 lighting = vec3(0.0);
    return lighting;
}

void main() {
    
    FragColor = texture(albedo, tcoord);
}

