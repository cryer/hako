#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos; 

// --- 新增：接收从顶点着色器传来的光源空间坐标 ---
in vec4 FragPosLightSpace; 

struct Light {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct DirLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// 不加灯光的话，模型加载的片段着色器直接声明uniform sampler2D texture_diffuse1
// 然后main函数FragColor = texture(texture_diffuse1, TexCoords);即可，mesh.h会自动处理
// 多的纹理,不用灯光，也用不到镜面光纹理贴图，所以也不需要声明texture_specular1

uniform Light light;
uniform DirLight dirLight;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform vec3 viewPos;

uniform float shininess;  // 镜面光的高光反光度

// 阴影贴图采样器 (建议绑定到较高的纹理单元，比如 10，避免和模型原本的纹理冲突)
uniform sampler2DShadow shadowMap;
uniform bool shadowOn;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, float shadow);

// 核心阴影计算函数
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    // 1. 执行透视除法 (将坐标转换到 NDC 空间 [-1, 1])
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 2. 变换到 [0,1] 的范围，以便对深度贴图进行采样
    projCoords = projCoords * 0.5 + 0.5;
    // 解决【过采样】: 当超出光源视锥体远平面时，强制没有阴影
    if(projCoords.z > 1.0)
        return 0.0;    
    // 3. 获取当前片元在光源视角的深度
    float currentDepth = projCoords.z;
    // 4. 解决【阴影失真(Shadow Acne)】: 动态计算偏移量 bias
    // 表面法线和光照方向夹角越大，bias越大。限制在一个最小最大区间。
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
    // 可以提高20帧左右
    // 返回值直接就是 0.0(在阴影中) 到 1.0(不在阴影中) 之间的插值
    float shadow = texture(shadowMap, vec3(projCoords.xy, currentDepth - bias));
    
    // 因为 sampler2DShadow 的返回值逻辑是：如果(当前深度 < 贴图深度) 返回1，否则返回0。
    // 所以我们需要反转一下，让 1 代表完全阴影。
    return 1.0 - shadow;
}

void main()
{   
     // 环境光
    vec3 ambient = light.ambient * texture(texture_diffuse1, TexCoords).rgb;

    // 漫反射 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(texture_diffuse1, TexCoords).rgb;

    // --- 新增：计算阴影值 ---
    // shadow为0代表被照亮，为1代表完全在阴影中
   float shadow = 0.0;
    if (shadowOn){
          shadow = ShadowCalculation(FragPosLightSpace, norm, lightDir);
    } 

    // 镜面光
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = light.specular * spec * texture(texture_specular1, TexCoords).rgb;  
    // 衰减
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));   

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    // vec3 result = ambient + diffuse + specular;
    // --- 新增：应用阴影 ---
    // 环境光不乘 (1.0 - shadow)，以便阴影处仍保留少许环境光泽，不至于死黑
    vec3 result = ambient + (1.0 - shadow) * (diffuse + specular);

    result += CalcDirLight(dirLight, norm, viewDir, shadow);

    FragColor = vec4(result, 1.0);

    // FragColor = texture(texture_diffuse1, TexCoords);
}


vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, float shadow)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    // combine results
    vec3 ambient = light.ambient *  texture(texture_diffuse1, TexCoords).rgb;
    vec3 diffuse = light.diffuse * diff * texture(texture_diffuse1, TexCoords).rgb;
    vec3 specular = light.specular * spec * texture(texture_specular1, TexCoords).rgb;
    vec3 result = ambient + (1.0 - shadow) * (diffuse + specular);
    return result ;
}
