#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 lightSpaceMatrix;
uniform mat4 model;
void main()
{
    //光源坐标系
    gl_Position = lightSpaceMatrix * model * vec4(aPos,1.0);
}