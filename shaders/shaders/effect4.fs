#version 330 core
out vec4 FragColor;

uniform vec2 iResolution;
uniform float iTime;

void main()
{
    vec4 o = vec4(0.0);
    vec2 uv = (gl_FragCoord.xy - iResolution.xy * 0.5) / iResolution.y;

    float z = 0.0; 
    float f = 0.0;
    float i = 0.0;

    for(i = 0.0; i < 40.0; i++)
    {
        vec3 p = vec3(uv * z, -z); 
        p.z -= iTime;        
        f = 1.0;
        for(float j = 1.0; j < 6.0; j++) 
        {
            p += sin(round(p * 6.0) / 6.0 * j - iTime * 0.5) / j;
        }

        float d1 = length(p.xy);
        float d2 = dot(cos(p), sin(p / 0.6).yzx);
        f = 0.003 + abs(d1 + d2 - 5.0) / 7.0;

        // z += f
        z += f;

        vec3 colorSrc = (1.5 - p / z) / f;
        o.g += colorSrc.x;
        o.r += colorSrc.y;
        o.b += colorSrc.z;
    }

    o = tanh(o / 3000.0);
    o.a = 1.0; 
    FragColor = o;
}