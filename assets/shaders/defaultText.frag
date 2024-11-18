#version 460 core

out vec4 FragColor;
in vec3 position;
in vec3 normal;
in vec2 tcoord;
in vec3 color;

uniform sampler2D albedo;

bool ditherValue(float brightness, vec2 pos) {
    
    // do the simple math first
    if (brightness > 16.0/17.0) return false;
    if (brightness < 01.0/17.0) return true;
    
    vec2 pixel = floor(mod((pos.xy+0.5), 4.0));
    int x = int(pixel.x);
    int y = int(pixel.y);
    bool result = false;
    
    // compute the 16 values by hand, store when it's a match
    	 if (x == 0 && y == 0) result = brightness < 16.0/17.0;
   	else if (x == 2 && y == 2) result = brightness < 15.0/17.0;
   	else if (x == 2 && y == 0) result = brightness < 14.0/17.0;
   	else if (x == 0 && y == 2) result = brightness < 13.0/17.0;
   	else if (x == 1 && y == 1) result = brightness < 12.0/17.0;
   	else if (x == 3 && y == 3) result = brightness < 11.0/17.0;
   	else if (x == 3 && y == 1) result = brightness < 10.0/17.0;
   	else if (x == 1 && y == 3) result = brightness < 09.0/17.0;
   	else if (x == 1 && y == 0) result = brightness < 08.0/17.0;
   	else if (x == 3 && y == 2) result = brightness < 07.0/17.0;
   	else if (x == 3 && y == 0) result = brightness < 06.0/17.0;
    else if (x == 0 && y == 1) result =	brightness < 05.0/17.0;
   	else if (x == 1 && y == 2) result = brightness < 04.0/17.0;
   	else if (x == 2 && y == 3) result = brightness < 03.0/17.0;
   	else if (x == 2 && y == 1) result = brightness < 02.0/17.0;
   	else if (x == 0 && y == 3) result = brightness < 01.0/17.0;
        
	return result;
}

void main() {
    
    vec4 textureColor = texture(albedo, tcoord);
    
    if(ditherValue(textureColor.r, gl_FragCoord.xy)) {
        discard;
    }

    FragColor = vec4(color.rgb, 1.0);
}

