#version 330 core
layout (location = 0) in vec3 aPos;
out vec3 TexCoords;
out vec4 FragPositionInLightSpace;//主要是绘制阴影使用的
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;
void main()
{
    TexCoords = aPos;//当作方向向量在cubeMap上采样
    // gl_Position = projection * view * model * vec4(aPos,1.0f);
    // gl_Position = projection * view * vec4(aPos,1.0f);
    vec4 pos = projection * view * model * vec4(aPos,1.0f);
    FragPositionInLightSpace = lightSpaceMatrix * pos;
    gl_Position = pos.xyww;
}