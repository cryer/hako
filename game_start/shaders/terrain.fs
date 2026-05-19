#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

in vec4 FragPosLightSpace;

uniform sampler2D terrainTexture;
uniform vec3 lightPos;
uniform vec3 viewPos;

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirLight dirLight;

uniform sampler2DShadow shadowMap;
uniform bool shadowOn;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 color, float shadow);

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
    vec3 color = texture(terrainTexture, fs_in.TexCoords).rgb;
    vec3 ambient = 0.08 * color;

    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    vec3 normal = normalize(fs_in.Normal);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;

    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
    vec3 specular = vec3(0.15) * spec;

    float shadow = 0.0;
    if (shadowOn){
        shadow = ShadowCalculation(FragPosLightSpace, normal, lightDir);
    }

    vec3 result = ambient + (1.0 - shadow) * (diffuse + specular);
    result += CalcDirLight(dirLight, normal, viewDir, color, shadow);

    FragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 color, float shadow)
{
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
    vec3 ambient = light.ambient * color;
    vec3 diffuse = light.diffuse * diff * color;
    vec3 specular = light.specular * spec * vec3(0.15);
    return ambient + (1.0 - shadow) * (diffuse + specular);
}
