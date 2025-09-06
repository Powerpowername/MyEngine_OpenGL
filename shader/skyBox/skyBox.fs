#version 330 core
out vec4 FragColor;

in vec3 TexCoords;
in vec4 FragPositionInLightSpace;
uniform samplerCube skybox;
uniform sampler2D shadowMap;
bool ShadowCalculation(vec4 fragPosLightSpace);
void main()
{
    if(ShadowCalculation(FragPositionInLightSpace))
        FragColor = 0.2 * texture(skybox,TexCoords);
    FragColor = texture(skybox,TexCoords);
}

bool ShadowCalculation(vec4 fragPosLightSpace)
{
    // 执行透视除法
    vec3 coords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 变换到[0,1]的范围
    coords = coords * 0.5 + 0.5;
    // 取得最近点的深度(使用[0,1]范围下的fragPosLight当坐标)
    float closestDepth = texture(shadowMap, coords.xy).r; 
    // 取得当前片段在光源视角下的深度
    float currentDepth = coords.z;
    // 检查当前片段是否在阴影中
    bool shadow = currentDepth > closestDepth  ? true : false;

    return shadow;
}