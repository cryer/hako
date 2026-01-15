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
    float t = iTime;

    for(i = 0.0; i < 50.0; i++)
    {
        vec3 p = z * rayDir;
        p.z += 7.0; 
        vec3 a = vec3(0.0);
        a.y = 1.0; 
        float h = length(p) - t;
        float s = sin(h);
        float c = cos(h);
        vec3 dotPart = dot(a, p) * a;
        vec3 crossPart = cross(a, p);
        a = mix(dotPart, p, s) + c * crossPart;

        float d_loop = 0.0;
        for(float j = 1.0; j <= 9.0; j++)
        {
            a += sin(round(a * j) - t).zxy / j;
        }
        float d_step = 0.1 * length(a.xz);
        z += d_step;

        if(d_step < 0.0001) d_step = 0.0001;
        o += vec4(3.0, z, i, 1.0) / d_step;
    }

    o = tanh(o / 10000.0);
    FragColor = o;
}