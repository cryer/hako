#version 330 core
out vec4 FragColor;

uniform vec2 iResolution;
uniform float iTime;

void main()
{
    vec4 o = vec4(0.0);
    vec2 uv = (gl_FragCoord.xy - iResolution.xy * 0.5) / iResolution.y;
    
    vec3 rayDir = normalize(vec3(uv, -1.0));
    
    float z = 0.0;
    float d = 0.0; 
    float i = 0.0;

    for(i = 0.0; i < 90.0; i++)
    {
        vec3 p = z * rayDir;
        
        p = vec3(atan(p.y, p.x), p.z / 8.0 - iTime, length(p.xy) - 9.0);
        
        for(float j = 1.0; j <= 7.0; j++)
        {
            p += sin(p.yzx * j + iTime + i * 0.2) / j;
        }
        float dist = 0.2 * length(vec4(0.2 * cos(6.0 * p) - 0.2, p.z));
        d = dist;
        z += d;
        vec4 colorShift = cos(p.x + vec4(0.0, 0.5, 1.0, 0.0)) + 1.0;
        
        float safeD = max(d, 0.001);
        float safeZ = max(z, 0.001);
        
        o += colorShift / safeD / safeZ;
    }

    o = tanh(o * o / 300.0); 
    FragColor = o;
}