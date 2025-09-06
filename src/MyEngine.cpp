#define STB_IMAGE_IMPLEMENTATION  
#include "MyEngine.h"
#include <cassert>
#include "stb_image/stb_image.h"
#pragma region
Object::~Object()
{
}

#pragma endregion

#pragma region QuaternionTransForm : public Object
// vec3 QuaternionTransForm::worldUp = vec3(0,1,0);
QuaternionTransForm::QuaternionTransForm(QuaternionTransForm::MoveObject moveObject = QuaternionTransForm::MoveObject::DEFAULT,
    vec3 position, vec3 forward,vec3 right, 
    float fovY, float aspectRatio, float nearZ, float farZ)
    :moveObject(moveObject),position(position),forward(forward),right(right),
    fovY(fovY),aspectRatio(aspectRatio),nearZ(nearZ),farZ(farZ),
    initForward(forward),initRight(right),
    initPosition(position)
{
    name = "QuaternionTransForm_";
    // rotationQuat = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    Update();//此处一定要更新project矩阵 view矩阵，up向量
}

QuaternionTransForm::~QuaternionTransForm()
{
}

void QuaternionTransForm::SetIsPerspective(bool isPerspective)
{
    perspective = isPerspective;
}

mat4 QuaternionTransForm::GetViewMatrixTransposed()
{
    return viewMatrixTransposed;
}

mat4 QuaternionTransForm::GetProjectionMatrixTransposed()
{
    return projMatrixTransposed;
}

mat4 QuaternionTransForm::GetModeMatrixTransposed()
{
    return modeMatrixTransposed;
}

void QuaternionTransForm::Update()
{
    //处理旋转逻辑
    if(selfRoationAxisFlag == false)
    {
        //自转
        Rotate();
    }
    else
    {
        SelfDefineAxisRotate();//计算自定义转轴选转的四元数
    }
    //更新局部坐标系

    rotationQuat = glm::normalize(rotationQuat);//归一化防止误差
    UpdateCoordinateSystem(rotationQuat);//更新局部坐标系

    if (moveObject == MoveObject::CAMERA)
    {
        //因为摄像机正常情况下是不需要传入mode矩阵的所以这里就没有处理了
        UpdateProjection();
        UpdateView();
    }

    else if(moveObject == MoveObject::RENDEROBJECT)
    {
        modeMatrixTransposed = mat4(1);
        //缩放->旋转->位移
        UpdataScle();
        modeMatrixTransposed = glm::mat4_cast(rotationQuat) * modeMatrixTransposed;//旋转
        UpdateTranslate();//更新位移modeMatrixTransposed


        UpdateProjection();
        UpdateView();
    }

}

void QuaternionTransForm::UpdateProjection()
{
    if(perspective)
        projMatrixTransposed = glm::perspective(glm::radians(fovY),aspectRatio,nearZ,farZ);
    else
        projMatrixTransposed = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f,nearZ,farZ);
}

void QuaternionTransForm::UpdateView()
{
    //lookAt函数的参数是targetPosition,是看向的目标点
    viewMatrixTransposed = glm::lookAt(position,position + forward,up);
}

void QuaternionTransForm::UpdataScle()
{
    modeMatrixTransposed = glm::scale(modeMatrixTransposed, scale);
}

/// @brief 根据位移变换位移矩阵
/// @param delta 位移量
void QuaternionTransForm::UpdateTranslate()
{
    vec3 deltaTranslate = position - initPosition;
    modeMatrixTransposed = glm::translate(modeMatrixTransposed, deltaTranslate);
}

void QuaternionTransForm::UpdateCoordinateSystem(quat rotateQuat)
{
    rotateQuat = glm::normalize(rotateQuat);//归一化防止误差

    mat3 rotationMat = glm::mat3_cast(rotateQuat); // 使用四元数生成旋转矩阵,glm::mat3(mQuaternionRotation)不行

    // 处理选转逻辑
    forward = glm::normalize(rotationMat * initForward);

    right = glm::normalize(rotationMat * initRight);
    // 因为现在增加了roll所以就不可以用下面的方式更新right了
    //  mRight = glm::normalize(glm::cross(mForward,worldUp));
    up = glm::normalize(glm::cross(right, forward));
}

void QuaternionTransForm::Rotate()
{
    //自转
    vec3 deltaRotation = targetRotation - initRotation;
    //yaw,pitch,roll的顺序
    rotationQuat = glm::quat(glm::radians(deltaRotation));
    
}
    

/// @brief 自定义转轴旋转
/// @param angle 弧度
/// @param axis 转轴
/// @return 旋转四元数
void QuaternionTransForm::SelfDefineAxisRotate()
{  
    float deltaRotation = slefDefineTargetRotate - slefDefineInitRotation;
    rotationQuat = glm::angleAxis(glm::radians(deltaRotation),slefDefineAxis);
}

void QuaternionTransForm::MouseMove(float xposIn,float yposIn)
{
    xposIn *= mouseSensitivity;
    yposIn *= mouseSensitivity;
    targetRotation.x += yposIn;
    targetRotation.y += xposIn;
}

void QuaternionTransForm::MouseScroll(float yoffset)
{
    //滑轮向前，视野变小，同样的显示区域内物体会变大
    fovY -= yoffset;
    if (fovY < 1.0f)
        fovY = 1.0f;
    if (fovY > 90.0f)
        fovY = 90.0f;
}

void QuaternionTransForm::KeyBoardMove(float delta)
{
    assert(moveObject != MoveObject::CAMERA);
    switch(movement)
    {
    case Movement::FORMWARD: 
        position += forward * delta;break;
    case Movement::BACKWARD:
        position += -(forward * delta);break;
    case Movement::LEFT:
        position += -(right * delta);break;
    case Movement::RIGHT:
        position += right * delta;break;
    }
    
}

void QuaternionTransForm::OnGUI()
{
    ImGui::DragFloat3((name + "Postion").c_str(), (float *)&position, 0.5f, -100.0f, 100.0f);

    if (moveObject == MoveObject::CAMERA)
    {
        ImGui::DragFloat((name + "FovY").c_str(),(float*)&fovY,0.05f, 1.0f, 90.0f);
        ImGui::DragFloat((name + "NearZ").c_str(),(float*)&nearZ,0.01f,0.1f,100.0f);
        ImGui::DragFloat((name + "FarZ").c_str(),(float*)&farZ,0.01f,0.1f,100.0f);
        ImGui::DragFloat((name + "MouseSensitivity").c_str(),(float*)&mouseSensitivity,0.01f,0.01f,100.0f);
    }

    if(moveObject == MoveObject::RENDEROBJECT)
    {
        ImGui::DragFloat3((name + "Scale").c_str(), (float *)&scale, 0.005f, 0.1f, 10.0f);
    }
    //判断是否启用自定义转轴
    ImGui::Checkbox((name + "EnableSelfRotationAxis").c_str(),&selfRoationAxisFlag);
    if(selfRoationAxisFlag)
    {
        //自定义转轴更新就重置初始旋转量
        if(ImGui::DragFloat3((name + "SelfRotationAxis").c_str(),(float *)&slefDefineAxis,0.05f,0.0f,100.0f))
        {
            //避免0,0,0转轴的未定义行为
            if(slefDefineAxis == vec3(0,0,0))
            {
                slefDefineAxis = vec3(1,0,0);
            }
            slefDefineInitRotation = 0;
        }
        //设置自定转轴旋转量
        ImGui::DragFloat((name + "SelfTargetRotate").c_str(),(float *)&slefDefineTargetRotate,0.05f,-180.0f,180.0f);
    }
    //自转yaw,pitch,roll
    else
    {
        ImGui::DragFloat3((name + "TargetRotation").c_str(),(float *)&targetRotation, 0.5f, -180.0f, 180.0f);
    }

}
#pragma endregion

#pragma region Input
// GLFWwindow* Input::window = nullptr;
unsigned char Input::status = 0;
float Input::currentFrameTime = 0;
float Input::lastFrameTime = 0;
bool Input::firstMouse = false;
void Input::GetKeyBoardInput()
{
    if(glfwGetKey(Setting::window,GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(Setting::window,true);

    if((glfwGetKey(Setting::window,GLFW_KEY_W) == GLFW_PRESS))
        SetWordW(true);
    else
        SetWordW(false);

    if((glfwGetKey(Setting::window,GLFW_KEY_S) == GLFW_PRESS))
        SetWordS(true);
    else
        SetWordS(false);

    if((glfwGetKey(Setting::window,GLFW_KEY_A) == GLFW_PRESS))
        SetWordA(true);
    else
        SetWordA(false);

    if((glfwGetKey(Setting::window,GLFW_KEY_D) == GLFW_PRESS))
        SetWordD(true);
    else
        SetWordD(false);   
}

bool Input::GetWordW()
{
    return (status & 8);//1000
}

bool Input::GetWordS()
{
    return (status & 4);//0100
}

bool Input::GetWordA()
{
    return (status & 2);//0010
}

bool Input::GetWordD()
{
    return (status & 1);//0001
}

float Input::DeltaFrameTime()
{
    return (currentFrameTime - lastFrameTime);
}

void Input::FreshFrameTime()
{
    lastFrameTime = currentFrameTime;
    currentFrameTime = static_cast<float>(glfwGetTime());
}

void Input::SetWordW(bool isPush)
{
    if(isPush)
        status = status | 8;//1000
    else
        status = status | 0;//0000
}

void Input::SetWordS(bool isPush)
{
    if(isPush)
        status = status | 4;//0100
    else
        status = status | 0;//0000
}

void Input::SetWordA(bool isPush)
{
    if(isPush)
        status = status | 2;//0010
    else
        status = status | 0;//0000
}

void Input::SetWordD(bool isPush)
{
    if(isPush)
        status = status | 1;//0001
    else
        status = status | 0;//0000
}

#pragma endregion

#pragma region QuaternionCamera : public Object
int QuaternionCamera::cameraNum = 0;
QuaternionCamera::QuaternionCamera(vec3 position, vec3 forward, vec3 right, float fovY, float aspectRatio, float nearZ, float farZ)
{
    ID = cameraNum++;
    name = "Camera_" + std::to_string(ID);
    
    transfrom = std::make_unique<QuaternionTransForm>();
    transfrom->name = name + "transform";
    transfrom->moveObject = QuaternionTransForm::MoveObject::CAMERA;
    transfrom->position =position;
    transfrom->forward =forward;
    transfrom->right =right;
    transfrom->fovY =fovY;
    transfrom->aspectRatio =aspectRatio;
    transfrom->nearZ =nearZ;
    transfrom->farZ =farZ;
}

void QuaternionCamera::SetCameraPosition(vec3 position)
{
    transfrom->position =position;
}

void QuaternionCamera::SetCameraFovY(float fovY)
{
    transfrom->fovY =fovY;
}

void QuaternionCamera::SetCameraAspectRatio(float aspectRatio)
{
    transfrom->aspectRatio =aspectRatio;
}

void QuaternionCamera::SetCameraNearZ(float nearZ)
{
    transfrom->nearZ =nearZ;
}

void QuaternionCamera::SetCameraFarZ(float farZ)
{
    transfrom->farZ =farZ;
}

QuaternionCamera::~QuaternionCamera()
{
}

void QuaternionCamera::OnGUI()
{   
    ImGui::Separator();
    if(ImGui::Checkbox("EnableMouseMove",&enableMouseMove));
    transfrom->OnGUI();
}

void QuaternionCamera::Update()
{
    transfrom->Update();
}

bool QuaternionCamera::ShowMouseMoveFlag()
{
    return enableMouseMove;
}

mat4 QuaternionCamera::GetViewMatrixTransposed()
{
    return transfrom->GetViewMatrixTransposed();
}

mat4 QuaternionCamera::GetProjectionMatrixTransposed()
{
    return transfrom->GetProjectionMatrixTransposed();
}

vec3 QuaternionCamera::GetCameraPosition()
{
    return transfrom->position;
}

QuaternionCamera::QuaternionCamera(QuaternionCamera &&other):Object(other),transfrom(std::move(other.transfrom))
{
    this->ID = other.ID;
}

#pragma endregion

#pragma region RenderObject : public Object

RenderObject::RenderObject(RenderObject &&other):
VBO(other.VBO),VAO(other.VAO),
diffuseMap(other.diffuseMap),normalMap(other.normalMap),tangentMap(other.tangentMap),bittangentMap(other.bittangentMap),
transform(std::move(transform)),Object(other)
{
}

RenderObject::~RenderObject()
{
    // 释放顶点数组对象（VAO）
    glDeleteVertexArrays(1, &VAO);
    // 释放顶点缓冲对象（VBO）
    glDeleteBuffers(1, &VBO);

    // 释放各类纹理（漫反射、法线、切线、副切线纹理）
    glDeleteTextures(1, &diffuseMap);
    glDeleteTextures(1, &normalMap);
    glDeleteTextures(1, &tangentMap);
    glDeleteTextures(1, &bittangentMap);

    // 释放后将 ID 置为 0，避免误用已释放的资源
    VAO = 0;
    VBO = 0;
    diffuseMap = 0;
    normalMap = 0;
    tangentMap = 0;
    bittangentMap = 0;
}

#pragma endregion

#pragma rergion Cube : public RenderObject
int Cube::cubeNum = 0;
Cube::Cube(vec3 position, vec3 forward, vec3 right):RenderObject()
{
    cubeID = cubeNum++;
    name = "Cube_" + std::to_string(cubeID);
    
    transform = std::make_unique<QuaternionTransForm>();
    transform->name = name + "transform";
    transform->moveObject = QuaternionTransForm::MoveObject::RENDEROBJECT;
    transform->position = position;
    transform->forward = forward;
    transform->right = right;
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

}

Cube::~Cube()
{
}

void Cube::RenderInit(unsigned int diffuseMap, unsigned int normalMap)
{
    this->diffuseMap = diffuseMap;
    this->normalMap = normalMap;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalMap);

    glBindVertexArray(VAO);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,11 * sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    //切线
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);
}

void Cube::Render(Shader& shader)
{
    shader.use();
    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalMap);
    glDrawArrays(GL_TRIANGLES,0,36);
    glBindVertexArray(0);
}

void Cube::OnGUI()
{
    ImGui::Separator();
    transform->OnGUI();
}

void Cube::Update()
{
    transform->Update();
}

void Cube::SetShader(Shader &shader,Object* viewObject)
{
    QuaternionTransForm* viewTransform = dynamic_cast<QuaternionTransForm*> (viewObject); 
    shader.use();
    //配置纹理啥的
	shader.setInt("material.texture_diffuse",0);
	shader.setInt("material.texture_normal",1);
	shader.setFloat("material.shininess",0.03);
    //传入model，proj，view等
    shader.setMat4("projection",viewTransform->GetProjectionMatrixTransposed());
    shader.setMat4("view",viewTransform->GetViewMatrixTransposed());
    shader.setMat4("model",transform->GetModeMatrixTransposed());
    shader.setVec3("viewPos", viewTransform->position);
    // std::cout<<Setting::MainCamera->GetCameraPosition().x<<" "<<Setting::MainCamera->GetCameraPosition().y<<" "<<Setting::MainCamera->GetCameraPosition().z<<std::endl;
    // lightSpaceMatrix

}

Cube::Cube(Cube &&other):RenderObject(std::move(other))
{
}

#pragma endregion


#pragma region AbstractLight : public Object 
// void AbstractLight::SetDepthResolution(unsigned int ShadowWidth, unsigned int ShadowHeight)
// {
//     this->ShadowWidth = ShadowWidth;
//     this->ShadowHeight = ShadowHeight;
// }

AbstractLight::~AbstractLight()
{ 
    // // 释放深度纹理（depthMap）
    // glDeleteTextures(1, &depthMap);
    // // 释放帧缓冲（depthMapFBO）
    // glDeleteFramebuffers(1, &depthMapFBO);
    // // 释放后建议将变量置为0，避免误用已释放的资源
    // depthMap = 0;
    // depthMapFBO = 0;
}

AbstractLight::AbstractLight(AbstractLight &&other):Object(std::move(other)),lightColor(other.lightColor)
// ,depthMap(other.depthMap),
// ShadowWidth(other.ShadowWidth),ShadowHeight(other.ShadowHeight)
{
}
#pragma endregion

#pragma region DirctionLight : public AbstractLight 
int DirctionLight::dirctionLightNum = 0;

DirctionLight::DirctionLight(vec3 lightColor, vec3 position, vec3 forward, vec3 right)
{
    dirctionLightID = dirctionLightNum++;
    name = "DirctionLight_" + std::to_string(dirctionLightID);
    transform = std::make_unique<QuaternionTransForm>();
    transform->name = name;
    transform->moveObject = QuaternionTransForm::MoveObject::RENDEROBJECT;
    transform->SetIsPerspective(false);//启用正射投影
    transform->position = position;
    transform->forward = forward;
    transform->right = right;
    this->lightColor = lightColor;
    //初始化s
    // glGenBuffers(1,&depthMapFBO);
    // glGenTextures(1,&depthMap);

}

// void DirctionLight::ConfigShadow(unsigned int depthMapFBO, unsigned int depthMap)
// {
//     this->depthMapFBO = depthMapFBO;
//     this->depthMap = depthMap;
//     glBindTexture(GL_TEXTURE_2D, depthMap);
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
//                  ShadowWidth, ShadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL); // 最后数据为NULL，不往深度纹理上填充初始化数据
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//     // 防止超过正是投影所造成的永远为阴影的情况，改变问题的环绕方式即可
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
//     // 设置边界颜色
//     float borderColor[] = {1.0, 1.0, 1.0, 1.0};
//     glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
//     // 将深度纹理附加到深度帧缓冲
//     glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO); // 帧缓冲
//     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
//     GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
//     if (status == GL_FRAMEBUFFER_COMPLETE)
//     {
//         std::cout << "帧缓冲完整" << status << std::endl;
//     }
//     else
//     {
//         switch (status)
//         {
//         case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
//             std::cout << "帧缓冲附件不完整" << std::endl;
//             break;
//         case GL_FRAMEBUFFER_UNSUPPORTED:
//             std::cout << "帧缓冲格式不受支持" << std::endl;
//             break;
//         // 其他情况可以继续添加
//         default:
//             break;
//         }
//     }
//     // 此处只要深度信息，不需要颜色缓冲
//     GLboolean isFBO = glIsFramebuffer(depthMapFBO);
//     std::cout << "depthMapFBO 是否为有效帧缓冲: " << (isFBO ? "是" : "否") << std::endl;
//     GLint currentFbo;
//     glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFbo);
//     std::cout << "绑定后的帧缓冲: " << currentFbo << "depthMapFBO: " << depthMapFBO << std::endl; // 应等于depthMapFBO的值

//     glDrawBuffer(GL_NONE);
//     glReadBuffer(GL_NONE);
//     // 配置完了就恢复默认帧缓冲
//     glBindFramebuffer(GL_FRAMEBUFFER, 0);
// }

void DirctionLight::SetShader(Shader &shader)
{
    //启用光源
    shader.use();
    shader.setBool("directionLight[" + std::to_string(dirctionLightID) + "].flag",1);
    shader.setVec3("directionLight[" + std::to_string(dirctionLightID) + "].color",lightColor);
    shader.setVec3("directionLight[" + std::to_string(dirctionLightID) + "].pos",transform->position);
    // std::cout<<"directionLight->position: "<<transform->position.x<< "+"<<transform->position.y<< "+"<<transform->position.z<<std::endl;
    shader.setVec3("directionLight[" + std::to_string(dirctionLightID) + "].front",-transform->forward);
    // std::cout<<"directionLight[" + std::to_string(dirctionLightID) + "].front"<<std::endl;
    // std::cout<<"directionLight->forward: "<<-transform->forward.x<< "+"<<-transform->forward.y<< "+"<<-transform->forward.z<<std::endl;
}  

// void DirctionLight::SetDepthShader(Shader& depthShader)
// {
//     //传入光源坐标系矩阵
//     depthShader.use();
//     mat4 lightSpaceMatrix = transform->GetProjectionMatrixTransposed() * transform->GetViewMatrixTransposed();
//     depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
// }

// void DirctionLight::RenderDepth(Shader& depthShader, std::function<void(Shader &)> objectRenderFunc)
// {
//     depthShader.use();
//     // glActiveTexture(GL_TEXTURE0);
//     // glBindTexture(GL_TEXTURE_2D,depthMap);
//     glBindFramebuffer(GL_FRAMEBUFFER,depthMapFBO);
//     glViewport(0,0,ShadowWidth,ShadowHeight);//其实设置的就是深度缓冲的分辨率



//     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//     //调用被绘制物体的shader，以光源的坐标空间绘制出深度纹理
//     objectRenderFunc(depthShader);
//     //恢复到默认帧缓冲
//     glBindFramebuffer(GL_FRAMEBUFFER,0);
//     //将视口恢复到实际窗口大小
//     glViewport(0,0,Setting::pWindowSize.x,Setting::pWindowSize.y);
// }
void DirctionLight::OnGUI()
{
    ImGui::Separator();
    transform->OnGUI();
    ImGui::DragFloat3((name + "LightColor").c_str(),(float*)&lightColor,0.05,0.0f,1.0f);
}
void DirctionLight::Update()
{
    transform->Update();
}
DirctionLight::DirctionLight(DirctionLight &&other) : AbstractLight(std::move(other)), dirctionLightID(other.dirctionLightID)
{
    ImGui::Separator();
    transform->OnGUI();
}
#pragma endregion

#pragma region PointLight : public AbstractLight 
int PointLight::pointLightNum = 0;
PointLight::PointLight(vec3 lightColor, vec3 position, vec3 forward, vec3 right)
{
    pointLightID = pointLightNum++;
    name = "PointLight" + std::to_string(pointLightID);
    transform = std::make_unique<QuaternionTransForm>();
    transform->name = name;
    transform->moveObject = QuaternionTransForm::MoveObject::RENDEROBJECT;
    transform->SetIsPerspective(false);//启用正射投影
    transform->position = position;
    transform->forward = forward;
    transform->right = right;
    this->lightColor = lightColor;
}
void PointLight::SetShader(Shader &shader)
{
    //启用光源
    shader.use();
    shader.setBool("pointLight[" + std::to_string(pointLightID) + "].flag",1);
    shader.setVec3("pointLight[" + std::to_string(pointLightID) + "].pos",transform->position);
    shader.setVec3("pointLight[" + std::to_string(pointLightID) + "].front",-transform->forward);
    // std::cout<<"pointLight->forward: "<<transform->forward.x<< "+"<<transform->forward.y<< "+"<<transform->forward.z<<std::endl;

    shader.setVec3("pointLight[" + std::to_string(pointLightID) + "].color",lightColor);
    shader.setFloat("pointLight[" + std::to_string(pointLightID) + "].constant",constant);
    shader.setFloat("pointLight[" + std::to_string(pointLightID) + "].linear",linear);
    shader.setFloat("pointLight[" + std::to_string(pointLightID) + "].quadratic",quadratic);
}
void PointLight::OnGUI()
{
    ImGui::Separator();
    transform->OnGUI();
    ImGui::DragFloat3((name + "LightColor").c_str(),(float*)&lightColor,0.05,0.0f,1.0f);
    ImGui::DragFloat((name + "Constant").c_str(),(float*)&constant,0.01,0.05f,10.0f);
    ImGui::DragFloat((name + "Linear").c_str(),(float*)&linear,0.01,0.05f,10.0f);
    ImGui::DragFloat((name + "Quadratic").c_str(),(float*)&quadratic,0.01,0.05f,10.0f);

}
void PointLight::Update()
{
    transform->Update();
}
PointLight::PointLight(PointLight &&other):AbstractLight(std::move(other))
{
    pointLightID = other.pointLightID;
    constant = other.constant;
    linear = other.linear;
    quadratic = other.quadratic;
    lightColor = other.lightColor;
    // other.pointLightID = 0;
}
#pragma endregion

#pragma region SpotLight : public AbstractLight 
int SpotLight::spotLightNum = 0;
SpotLight::SpotLight(vec3 lightColor, vec3 position, vec3 forward, vec3 right)
{
    spotLightID = spotLightNum++;
    name = "SpotLight" + std::to_string(spotLightID);
    transform = std::make_unique<QuaternionTransForm>();
    transform->name = name;
    transform->moveObject = QuaternionTransForm::MoveObject::RENDEROBJECT;
    transform->SetIsPerspective(false);//启用正射投影
    transform->position = position;
    transform->forward = forward;
    transform->right = right;
    this->lightColor = lightColor;
}

void SpotLight::SetShader(Shader &shader)
{
    shader.use();
    shader.setBool("spotLight[" + std::to_string(spotLightID) + "].flag",1);
    shader.setVec3("spotLight[" + std::to_string(spotLightID) + "].pos",transform->position);
    shader.setVec3("spotLight[" + std::to_string(spotLightID) + "].front",-transform->forward);
    shader.setVec3("spotLight[" + std::to_string(spotLightID) + "].color",lightColor);
    shader.setFloat("spotLight[" + std::to_string(spotLightID) + "].constant",constant);
    shader.setFloat("spotLight[" + std::to_string(spotLightID) + "].linear",linear);
    shader.setFloat("spotLight[" + std::to_string(spotLightID) + "].quadratic",quadratic);
    shader.setFloat("spotLight[" + std::to_string(spotLightID) + "].cosPhyInner",glm::cos(glm::radians(cosPhyInner)));
    shader.setFloat("spotLight[" + std::to_string(spotLightID) + "].quadratic",glm::cos(glm::radians(cosPhyOuter)));

}

void SpotLight::OnGUI()
{
    transform->OnGUI();
    ImGui::DragFloat((name + "Constant").c_str(), (float *)&constant, 0.1f, 0.0f, 2.0f);
    ImGui::DragFloat((name + "Linear").c_str(), (float *)&linear, 0.01f, 0.0f, 2.0f);
    ImGui::DragFloat((name + "Quadratic").c_str(), (float *)&quadratic, 0.01f, 0.0f, 2.0f);
    ImGui::DragFloat((name + "CosPhyInner").c_str(), (float *)&cosPhyInner, 1.0f, 0, 180.0f);
    ImGui::DragFloat((name + "CosPhyOuter").c_str(), (float *)&cosPhyOuter, 1.0f, 0, 180.0f);
}

void SpotLight::Update()
{
    transform->Update();
}

SpotLight::SpotLight(SpotLight &&other):AbstractLight(std::move(other))
{
    spotLightID = other.spotLightID;
    constant = other.constant;
    linear = other.linear;
    quadratic = other.quadratic;
    cosPhyInner = other.cosPhyInner;
    cosPhyOuter = other.cosPhyOuter;
    lightColor = other.lightColor;
}

#pragma endregion

QuaternionCamera* Setting::MainCamera = nullptr;
GLFWwindow* Setting::window = nullptr;
glm::vec2 Setting::pWindowSize = vec2(0,0);//真正的窗口尺寸

#pragma region 窗口事件及图像处理模块
/// @brief 
/// @param path 图片路径
/// @param reverse 判断是否延x轴翻转
/// @return 纹理对象的ID
unsigned int loadTexture(const char*  path,bool reverse)
{
    unsigned int textureID;
    glGenTextures(1,&textureID);
    int width,height,nrComponents;
    stbi_set_flip_vertically_on_load(reverse); 
    unsigned char* data = stbi_load(path,&width,&height,&nrComponents,0);
    if(data)
    {
        GLenum format;
        if(nrComponents == 1)
            format = GL_RED;
        else if(nrComponents == 3)
            format = GL_RGB;
        else if(nrComponents == 4)
            format = GL_RGBA;
        glBindTexture(GL_TEXTURE_2D,textureID);
        glTexImage2D(GL_TEXTURE_2D,0,format,width,height,0,format,GL_UNSIGNED_BYTE,data);
        glGenerateMipmap(GL_TEXTURE_2D);//生成多级渐近纹理

        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    //此处可以看出纹理对象开辟的内存（glGenTextures）实在堆区开辟的，这个id是在栈区的对象，被return之后就会被销毁，但是这块是传值出去的，所以不要紧
    return textureID;
}



#pragma endregion


#pragma region SkyBox : publicObject
int SkyBox::SkyBoxNum = 0;
SkyBox::SkyBox()
{
    SkyBoxID = SkyBoxNum++;
    name = "SkyBox" + std::to_string(SkyBoxID);
    transfrom = std::make_unique<QuaternionTransForm>();
    transfrom->moveObject = QuaternionTransForm::MoveObject::RENDEROBJECT;
    transfrom->name = name;
    //初始化OpenGL相关参数 
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);
    glGenBuffers(1,&IBO);
    glGenTextures(1,&textureID);
}
SkyBox::~SkyBox()
{
}
void SkyBox::RenderInit(const std::vector<string>& faces)
{  
    //绑定cubeMap纹理数据
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP,textureID);
    int width = 0,height = 0,nrComponents = 0;
    // stbi_set_flip_vertically_on_load(true);//立方体贴图的纹理是不需要反转的，他的内部是右做处理的
    for(unsigned int i = 0;i < faces.size();i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
        GLenum format;
        switch(nrComponents)
        {
        case 1:format = GL_RED;break;
        case 3:format = GL_RGB;break;
        case 4:format = GL_RGBA;break;
        default:std::cout<<"nrComponents error"<<std::endl;break;
        }
        if(data)
        {
            //右左上下前后
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,0,format,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
        
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);//强制取边缘纹理
    //绑定顶点数组缓冲，顶点缓冲，索引缓冲
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(indices),indices,GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);

    
    // 解绑
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
void SkyBox::SetShader(Shader &shader,Object* viewObject)
{
    QuaternionTransForm* viewTransform = dynamic_cast<QuaternionTransForm*>(viewObject);
    shader.use();
    shader.setMat4("model",transfrom->GetModeMatrixTransposed());
    shader.setMat4("view",mat4(mat3(viewTransform->GetViewMatrixTransposed())));//干掉位移操作
    // shader.setMat4("view",transform->GetViewMatrixTransposed());
    // shader.setMat4("view",m);//干掉位移操作

    // std::cout<<transform->up.x<<" "<< transform->up.y<<" "<<transform->up.z<<" "<<std::endl;
    // mat4 mat = Setting::MainCamera->transfrom->GetViewMatrixTransposed();
    // std::cout<<mat[0][0]<<"      "<<mat[0][1]<<"      "<<mat[0][2]<<"      "<<mat[0][3]<<"      "<<std::endl;
    // std::cout<<mat[1][0]<<"      "<<mat[1][1]<<"      "<<mat[1][2]<<"      "<<mat[1][3]<<"      "<<std::endl;
    // std::cout<<mat[2][0]<<"      "<<mat[2][1]<<"      "<<mat[2][2]<<"      "<<mat[2][3]<<"      "<<std::endl;
    // std::cout<<mat[3][0]<<"      "<<mat[3][1]<<"      "<<mat[3][2]<<"      "<<mat[3][3]<<"      "<<std::endl;
    shader.setMat4("projection",viewTransform->GetProjectionMatrixTransposed());
    shader.setVec3("cameraPos",viewTransform->position);
    shader.setInt("skybox", 0); 
    shader.setInt("shadowMap",1);
}
void SkyBox::Render(Shader &shader)
{
    
    glDepthFunc(GL_LEQUAL);
    // glDisable(GL_CULL_FACE); // 禁用面剔除，天空盒需要两面都渲染,其实在天空盒里面，可以开启面剔除的
    shader.use();
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,IBO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP,textureID);
    // glActiveTexture(GL_TEXTURE1);
    // glBindTexture(GL_TEXTURE_2D,depthMap);
    glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_INT,0);
    // glDrawArrays(GL_TRIANGLES, 0, 36);
    // 解绑VAO
    glBindVertexArray(0);
    // 恢复状态
    glDepthFunc(GL_LESS);
}
void SkyBox::OnGUI()
{
    //可以修改为动态加载立方体贴图的效果
    ImGui::Separator();
    transfrom->OnGUI();
}

void SkyBox::Update()
{
    transfrom->Update();
}
SkyBox::SkyBox(SkyBox && other):Object(std::move(other)),
SkyBoxID(other.SkyBoxID),VBO(other.VBO),VAO(other.VAO),IBO(other.IBO),textureID(other.textureID),
transfrom(std::move(other.transfrom))
{
}
#pragma endregion



