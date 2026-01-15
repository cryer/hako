#version 330 core
out vec4 FragColor;

uniform vec2 iResolution;
uniform float iTime;

mat2 rot(float a) {
    float s = sin(a);
    float c = cos(a);
    return mat2(c, -s, s, c);
}

void main()
{
    vec2 uv = (gl_FragCoord.xy - 0.5 * iResolution.xy) / iResolution.y;
    vec3 rayDir = normalize(vec3(uv, 1.0));
    
    vec3 p;   
    vec4 col = vec4(0.0); 
    
    float totalDist = 0.0;

    vec3 camPos = vec3(0.0, 0.0, -3.0 + iTime * 2.0); 

    for(float i = 0.0; i < 64.0; i++) {
        p = camPos + rayDir * totalDist;
        p.xy *= rot(p.z * 0.1); 
        
        vec3 q = mod(p, 2.0) - 1.0; 
        
        float d = length(q.xz) - 0.1; 
        d = min(d, length(q.xy) - 0.1); 
        
        vec3 neon = vec3(0.2, 0.8, 1.0); 
        col.rgb += neon * 0.02 / (abs(d) + 0.001) * exp(-0.05 * totalDist);
        
        float stepSize = max(0.1, abs(d)); 
        totalDist += stepSize * 0.5; 
    }

    col = 1.0 - exp(-col * 2.0); 
    FragColor = col;
}