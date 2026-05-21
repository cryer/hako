#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 sunDir;
uniform samplerCube skybox;
uniform sampler2DShadow shadowMap;
uniform bool shadowOn;
uniform float time;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if(projCoords.z > 1.0) return 0.0;
    float currentDepth = projCoords.z;
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);
    float shadow = texture(shadowMap, vec3(projCoords.xy, currentDepth - bias));
    return 1.0 - shadow;
}

void main()
{
    float nx = sin(FragPos.x * 3.5 + time * 0.7) * 0.25 + sin(FragPos.z * 2.8 + time * 1.1) * 0.20;
    float nz = cos(FragPos.z * 3.2 + time * 0.9) * 0.25 + cos(FragPos.x * 2.6 + time * 0.95) * 0.20;
    vec3 normal = normalize(vec3(-nx, 1.0, -nz));

    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 reflectDir = reflect(-viewDir, normal);
    vec3 reflectColor = texture(skybox, reflectDir).rgb;

    float fresnel = pow(1.0 - abs(dot(normal, viewDir)), 3.0);
    fresnel = clamp(fresnel, 0.0, 1.0);

    vec3 deepColor  = vec3(0.02, 0.12, 0.22);
    vec3 waterColor = vec3(0.08, 0.24, 0.38);
    vec3 baseColor  = mix(waterColor, deepColor, 0.45);

    vec3 color = mix(baseColor, reflectColor, fresnel * 0.75);

    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 halfway = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfway), 0.0), 256.0);
    color += vec3(0.55, 0.65, 0.75) * spec * 0.7;

    vec3 sunLightDir = normalize(-sunDir);
    vec3 sunHalfway = normalize(sunLightDir + viewDir);
    float sunSpec = pow(max(dot(normal, sunHalfway), 0.0), 512.0);
    color += vec3(0.75, 0.78, 0.70) * sunSpec * 0.45;

    float shadow = 0.0;
    if (shadowOn) {
        shadow = ShadowCalculation(FragPosLightSpace, normal, lightDir);
    }
    color *= (1.0 - shadow * 0.4);

    float alpha = mix(0.55, 0.82, fresnel);

    FragColor = vec4(color, alpha);
}
