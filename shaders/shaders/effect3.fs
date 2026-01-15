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
    float i = 0.0;
    float d = 0.0;

    for(i = 0.0; i < 55.0; i++)
    {
        vec3 p = z * rayDir;
        
        for(float j = 1.0; j <= 9.0; j++)
        {
            p += sin(ceil(p / 9.0).zxy + iTime);
        }
        d = 0.1 * abs(p.z + 40.0);
        z += d;

        float safeD = max(d, 0.002);
        o += vec4(9.0, 4.0, 2.0, 1.0) / safeD;
    }

    o = tanh(o / 6000.0);
    FragColor = o;
}