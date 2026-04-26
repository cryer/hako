#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

// --- 新增：接收从顶点着色器传来的光源空间坐标 ---
in vec4 FragPosLightSpace;

uniform sampler2D floorTexture;
uniform vec3 lightPos;
uniform vec3 viewPos;

// --- 新增：阴影贴图采样器 ---
uniform sampler2DShadow shadowMap;
uniform bool shadowOn;

// ==========================================
// 阴影计算核心函数 (模型片段着色器逻辑完全一致)
// ==========================================
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    if(projCoords.z > 1.0)
        return 0.0;
        
    float currentDepth = projCoords.z;
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);

    // float shadow = 0.0;
    // vec2 texelSize = 1.0 / textureSize(shadowMap, 0); 
    // for(int x = -1; x <= 1; ++x)
    // {
    //     for(int y = -1; y <= 1; ++y)
    //     {
    // 如果使用旧方法 需要sampler2D采样器而不是sampler2DShadow
    //         float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
    //         shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
    //     }    
    // }
    // shadow /= 9.0;
    
    // return shadow;
    
    // 5. 删掉那个昂贵的 for 循环！直接交给 texture 函数！
    // 注意 sampler2DShadow 的 texture 函数需要传 vec3，第三个参数是要比较的深度值
    // 返回值直接就是 0.0(在阴影中) 到 1.0(不在阴影中) 之间的插值
    float shadow = texture(shadowMap, vec3(projCoords.xy, currentDepth - bias));
    
    // 因为 sampler2DShadow 的返回值逻辑是：如果(当前深度 < 贴图深度) 返回1，否则返回0。
    // 所以我们需要反转一下，让 1 代表完全阴影。
    return 1.0 - shadow;
}

void main()
{           
    vec3 color = texture(floorTexture, fs_in.TexCoords).rgb;
    
    // ambient
    vec3 ambient = 0.05 * color;
    
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    vec3 normal = normalize(fs_in.Normal);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    // 这里使用 Blinn-Phong 的 halfwayDir
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = vec3(0.3) * spec; // 假设高光颜色为白色亮度0.3

    // --- 新增：计算阴影值 ---
    float shadow = 0.0;
    if (shadowOn){
          shadow = ShadowCalculation(FragPosLightSpace, normal, lightDir);
    } 
    

    // --- 新增：应用阴影 ---
    // 同样，只有漫反射和高光受遮挡影响，环境光不受影响
    vec3 result = ambient + (1.0 - shadow) * (diffuse + specular);

    FragColor = vec4(result, 1.0);
}