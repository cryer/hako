#version 330 core
out vec4 FragColor;

uniform vec2 iResolution; // 对应代码中的 r
uniform float iTime;      // 对应代码中的 t

void main()
{
    // 初始化输出颜色 o
    vec4 o = vec4(0.0);
    
    // 标准化UV坐标，中心为(0,0)
    // 对应原代码 normalize(FC.rgb*2.-r.xyy) 的逻辑
    // 原代码暗示了一个 vec3(uv, -1.0) 的视角方向（假设 r.xyy 的构造）
    vec2 uv = (gl_FragCoord.xy - iResolution.xy * 0.5) / iResolution.y;
    vec3 rayDir = normalize(vec3(uv, -1.0));

    // 初始化变量
    float z = 0.0; // 射线行进距离
    float i = 0.0; // 迭代次数
    float t = iTime;

    // 主循环：对应 for(float i,z,d,h;i++<5e1;...)
    for(i = 0.0; i < 50.0; i++)
    {
        // 计算当前空间点 p
        vec3 p = z * rayDir;
        p.z += 7.0; // 相机偏移

        // 初始化累加向量 a
        // 原代码: vec3 p=..., a; a.y++;
        vec3 a = vec3(0.0);
        a.y = 1.0; 

        // 旋转与形变核心逻辑
        // 原代码: a=mix(dot(a,p)*a,p,sin(h=length(p)-t))+cos(h)*cross(a,p);
        float h = length(p) - t;
        float s = sin(h);
        float c = cos(h);
        
        // 这是一个非常紧凑的罗德里格旋转公式变体
        // mix(x, y, a) = x*(1-a) + y*a
        vec3 dotPart = dot(a, p) * a;
        vec3 crossPart = cross(a, p);
        a = mix(dotPart, p, s) + c * crossPart;

        // 内部细节循环 (FBM-like 结构)
        // 原代码: for(d=0.;d++<9.;a+=sin(round(a*d)-t).zxy/d);
        // 注意：原代码 d 从 0 开始自增，第一次循环体内 d 为 1
        float d_loop = 0.0;
        for(float j = 1.0; j <= 9.0; j++)
        {
            a += sin(round(a * j) - t).zxy / j;
        }

        // 计算步进距离 (Density/Distance)
        // 原代码: z+=d=.1*length(a.xz);
        // 这里 d 被赋值为步进距离
        float d_step = 0.1 * length(a.xz);
        z += d_step;

        // 颜色累加
        // 原代码: o+=vec4(3,z,i,1)/d
        // 红色通道固定，绿色随深度增加，蓝色随迭代次数增加(产生发光感)
        // 除以 d_step 产生体积光效果（密度越大，d_step越小，颜色越亮）
        // 防止除以0 (虽然在此数学模型下极少发生)
        if(d_step < 0.0001) d_step = 0.0001;
        o += vec4(3.0, z, i, 1.0) / d_step;
    }

    // 色调映射
    // 原代码: o=tanh(o/1e4);
    // tanh 将高动态范围的值压缩到 [0,1]
    o = tanh(o / 10000.0);

    FragColor = o;
}