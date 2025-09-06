#version 330 core
#define NR_POINT_LIGHTS 4 //光源数量
struct Material
{
    sampler2D texture_diffuse;
    sampler2D texture_specular;
    sampler2D texture_normal;//normal本身就是切线空间内的坐标
    float shininess;
};

struct DirectionLight//平行光
{
    bool flag;
    vec3 color;
	vec3 pos;
	vec3 front;//光源前方向向量
};

struct PointLight//点光源
{
	bool flag; 
	vec3 pos;
	vec3 front;
    vec3 color;
    //衰减参数
	float constant;
	float linear;
	float quadratic;
};

struct SpotLight//聚光(手电筒)
{
	bool flag;
	vec3 pos;
	vec3 front;
    vec3 color;
    //衰减参数
	float constant;
	float linear;
	float quadratic;

    //聚光范围
	float cosPhyInner;//内圈
	float cosPhyOuter;//外圈
};



uniform Material material;
uniform DirectionLight directionLight[NR_POINT_LIGHTS];
uniform PointLight pointLight[NR_POINT_LIGHTS];
uniform SpotLight spotLight[NR_POINT_LIGHTS];

out vec4 FragColor;
in VS_OUT {
	vec2 TexCoords;

	vec3 ViewPos;
	vec3 FragPos;
	mat3 TBN;
} fs_in;

vec3 CalcDirectionLight(DirectionLight light,vec3 uNormal,vec3 fragToCameraDir);
vec3 CalcPointLight(PointLight light,vec3 uNormal,vec3 fragToCameraDir);
vec3 CalcSpotLight(SpotLight light,vec3 uNormal,vec3 fragToCameraDir);

void main()
{
	vec3 color = vec3(0,0,0);
	vec3 uNormal = normalize(texture(material.texture_normal,fs_in.TexCoords).rgb * 2.0f - 1.0f);
	uNormal = normalize(fs_in.TBN * uNormal);//转到世界坐标系

	vec3 fragToCameraDir  = normalize(fs_in.ViewPos - fs_in.FragPos);
	for(int i = 0;i < NR_POINT_LIGHTS;i++)
	{
		if(directionLight[i].flag)
			color+=CalcDirectionLight(directionLight[i],uNormal,fragToCameraDir);
		
		if(pointLight[i].flag)
			color+=CalcPointLight(pointLight[i],uNormal,fragToCameraDir);
		
		if(spotLight[i].flag)
			color+=CalcSpotLight(spotLight[i],uNormal,fragToCameraDir);
	}
	// FragColor = texture(material.texture_diffuse,fs_in.TexCoords);
	FragColor=vec4(color,1.0f);
}

//漫反射不需要考虑半程向量，镜面光才需要，环境光也不需要
vec3 CalcDirectionLight(DirectionLight light,vec3 uNormal,vec3 fragToCameraDir)
{
	//漫反射
	// vec3 fragToLightDir = normalize(-light.front);
	// vec3 fragToLightDir = vec3(-0.206506,0.0284836,-0.978031);
	// vec3 fragToLightDir = vec3(1,0,0);

	float diffIntensity = max(0,dot(normalize(-light.front),uNormal));//漫反射强度
	vec3 diffColor = diffIntensity * light.color * texture(material.texture_diffuse,fs_in.TexCoords).rgb;

	//镜面反射
	vec3 halfwayDir = normalize(normalize(-light.front) + fragToCameraDir);
	float specIntensity = pow(max(dot(uNormal,halfwayDir),0.0),material.shininess);
	vec3 specularColor = specIntensity * light.color * texture(material.texture_specular,fs_in.TexCoords).rgb;
	return diffColor + specularColor;

}

//有衰减的点光源
vec3 CalcPointLight(PointLight light,vec3 uNormal,vec3 fragToCameraDir)
{
	float dist = length(light.pos - fs_in.FragPos);
	float attenuation = 1.0f/(light.constant + light.linear * dist + light.quadratic * (dist * dist));

	//diffuse
	vec3 fragToLightDir = normalize(light.pos - fs_in.FragPos);
	float diffIntensity = max(0,dot(fragToLightDir,uNormal));
	vec3 diffColor = diffIntensity * light.color * texture(material.texture_diffuse,fs_in.TexCoords).rgb;

	//specular
	vec3 halfwayDir = normalize(fragToLightDir + fragToCameraDir);
	float specIntensity = pow(max(0,dot(uNormal,halfwayDir)),material.shininess);
	vec3 specularColor = specIntensity * light.color * texture(material.texture_specular,fs_in.TexCoords).rgb;

	return attenuation * (diffColor + specularColor);
}

//柔和聚光
vec3 CalcSpotLight(SpotLight light,vec3 uNormal,vec3 fragToCameraDir)
{
	vec3 fragToLightDir = normalize(light.pos - fs_in.FragPos);

	float dist = length(light.pos - fs_in.FragPos);
	float attenuation = 1.0f/(light.constant + light.linear * dist + light.quadratic * (dist * dist));

	float theta = dot(normalize(fs_in.FragPos - light.pos),-light.front);
	float epsilon = (light.cosPhyInner - light.cosPhyOuter);
	float intensity = clamp((theta - light.cosPhyOuter) / epsilon, 0.0, 1.0);//clamp是钳位函数或限幅函数，将值限定在0-1之间

	//diffuse
	float diffIntensity = max(dot(fragToLightDir,uNormal),0) * intensity;
	vec3 diffColor = diffIntensity * light.color * texture(material.texture_diffuse,fs_in.TexCoords).rgb;

	//specular
	vec3 halfwayDir = normalize(fragToLightDir + fragToCameraDir);
	float specIntensity = pow(max(0,dot(uNormal,halfwayDir)),material.shininess) * intensity;
	vec3 specularColor = specIntensity * light.color * texture(material.texture_specular,fs_in.TexCoords).rgb;

	return attenuation * (diffColor + specularColor);
}


