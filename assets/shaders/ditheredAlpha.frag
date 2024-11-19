#version 460 core

out vec4 FragColor;
in vec3 position;
in vec3 normal;
in vec2 tcoord;
in float time;

uniform sampler2D albedo;

vec3 hash32(vec2 p) {
	vec3 p3 = vec3(p.xyx) * vec3(443.8975, 397.2973, 491.1871);
	p3 = p3 - floor(p3);
    p3 += dot(p3, p3.yxz + 19.19);
	vec3 r3 = vec3((p3.x + p3.y) * p3.z, (p3.x + p3.z) * p3.y, (p3.y + p3.z) * p3.x);
	r3 = r3 - floor(r3);
	return r3;
}

void main() {
    
    vec4 color = texture(albedo, tcoord);
    
    float scaledDepth = (4.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near);
	float clipDepth = scaledDepth / gl_FragCoord.w;
    
	vec3 rand_v3 = hash32(gl_FragCoord.xy + vec2(time - floor(time), time - floor(time)));
    
    if(clipDepth < rand_v3.r || color.a < rand_v3.r) {
        discard;
    }

    FragColor = vec4(color.rgb, 1.0);
}

