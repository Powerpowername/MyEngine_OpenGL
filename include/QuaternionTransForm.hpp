#pragma once
#include <memory>
#include <string>
#include "imgui.h"
#include "glm/glm.hpp"
#include <glm/gtc/quaternion.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glad/glad.h"
#include "GLFW/glfw3.h"//glfw需要使用glad的定义，所以要在gald之后
#include <functional>
#include "shader.h"
#include <iostream>
#include <vector>
#include "stb_image/stb_image.h"
using std::string,std::unique_ptr,std::make_unique,std::cout,std::endl,std::vector;
using glm::vec3,glm::vec2,glm::mat3,glm::mat4,glm::quat;

#pragma region Object
class Object
{
public:
    //拷贝构造、拷贝赋值、析构函数均未显式定义，且未显式声明移动操作，才会自动生成移动构造函数
    string name;
    //function
    Object() = default;
    //纯虚析构函数必须要有定义，不然没法释放资源
    virtual ~Object(){};
    virtual void OnGUI() = 0;
    virtual void Update() = 0;
};
#pragma endregion

#pragma region QuaternionTransForm : public Object
//包含摄像机与普通绘制物体的四元数实现的旋转变化与位移变化
class QuaternionTransForm : public Object
{
protected:
    //是否是透视投影
    bool perspective = true;
    //旋转
    quat rotationQuat{1,0,0,0};//旋转四元数
    vec3 slefDefineAxis{1,0,0};//自定义转轴,给定一个初始值x轴，防止0，0，0的未定义行为
    float slefDefineInitRotation = 0;//自定以转轴的初始旋转量，一般为0
    float slefDefineTargetRotate = 0;//自定义转轴的目标值
    float mouseSensitivity = 0.1f;
    vec3 initPosition{0,0,0};//初始位置
    vec3 initRotation{0,0,0};//初始旋转角度（先旋转在位移的标准实现吗，也就是说物体在世界坐标系上旋转之后再进行位移），一般为0
    vec3 targetRotation{0,0,0};
    void Rotate();//计算世界坐标系绕XYZ轴旋转的四元数,之后再位移即可实现自传
    void SelfDefineAxisRotate();//计算自定义转轴选转的四元数

    //更新矩阵
    void UpdateProjection();
    void UpdateView();
    void UpdataScle();
    void UpdateTranslate();
    void UpdateCoordinateSystem(quat rotateQuat);
    mat4 modeMatrixTransposed{1};
    mat4 viewMatrixTransposed{1};
    mat4 projMatrixTransposed{1};

public: 
    //移动对象类型
    enum class MoveObject
    {
        CAMERA,
        RENDEROBJECT,
        DEFAULT
    } moveObject;

    //移动方向
    enum class Movement
    {
        FORMWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
    } movement;

    bool enbaleMouse = false;
    //自身坐标系相关属性
    vec3 position{0,0,0};
    vec3 right{0,0,0};
    vec3 up{0,0,0};
    vec3 forward{0,0,0};
    vec3 initForward{0,0,0};//选转前向量初始值
    vec3 initRight{0,0,0};//选转右向量初始值
    float fovY = 0;//视角
    float aspectRatio = 0;//高宽比
    float nearZ = 0;
    float farZ = 0;
    
    //旋转
    bool selfRoationAxisFlag = true;// 是否为自转标志位
    //缩放
    vec3 scale = vec3(1,1,1);

    //function
    QuaternionTransForm(QuaternionTransForm::MoveObject moveObject = QuaternionTransForm::MoveObject::DEFAULT,vec3 position = vec3(0,0,0),vec3 forward = vec3(0,0,-1),vec3 right = vec3(1,0,0),float fovY = 90.0f,float aspectRatio = 16.0f/9.0f,float nearZ = 0.1f,float farZ = 100.0f);
    ~QuaternionTransForm() override;
    void SetIsPerspective(bool isPerspective);
    mat4 GetViewMatrixTransposed();
    mat4 GetProjectionMatrixTransposed();
    mat4 GetModeMatrixTransposed();
    void MouseMove(float xposIn,float yposIn);//鼠标移动的回调事件
    void MouseScroll(float yoffset);
    void KeyBoardMove(float delta);//CAMERA的移动方式
    void OnGUI() override;
    void Update() override;
}; 
#pragma endregion


#pragma region QuaternionTransForm : public Object
// vec3 QuaternionTransForm::worldUp = vec3(0,1,0);
inline QuaternionTransForm::QuaternionTransForm(QuaternionTransForm::MoveObject moveObject,
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

inline QuaternionTransForm::~QuaternionTransForm()
{
}

inline void QuaternionTransForm::SetIsPerspective(bool isPerspective)
{
    perspective = isPerspective;
}

inline mat4 QuaternionTransForm::GetViewMatrixTransposed()
{
    return viewMatrixTransposed;
}

inline mat4 QuaternionTransForm::GetProjectionMatrixTransposed()
{
    return projMatrixTransposed;
}

inline mat4 QuaternionTransForm::GetModeMatrixTransposed()
{
    return modeMatrixTransposed;
}

inline void QuaternionTransForm::Update()
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

inline void QuaternionTransForm::UpdateProjection()
{
    if(perspective)
        projMatrixTransposed = glm::perspective(glm::radians(fovY),aspectRatio,nearZ,farZ);
    else
        projMatrixTransposed = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f,nearZ,farZ);
}

inline void QuaternionTransForm::UpdateView()
{
    //lookAt函数的参数是targetPosition,是看向的目标点
    viewMatrixTransposed = glm::lookAt(position,position + forward,up);
}

inline void QuaternionTransForm::UpdataScle()
{
    modeMatrixTransposed = glm::scale(modeMatrixTransposed, scale);
}

/// @brief 根据位移变换位移矩阵
/// @param delta 位移量
inline void QuaternionTransForm::UpdateTranslate()
{
    //平移
    vec3 deltaTranslate = position - initPosition;
    // modeMatrixTransposed = glm::translate(modeMatrixTransposed, deltaTranslate);
    modeMatrixTransposed = glm::translate(glm::mat4(1.0f), deltaTranslate) * modeMatrixTransposed;
}

inline void QuaternionTransForm::UpdateCoordinateSystem(quat rotateQuat)
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

inline void QuaternionTransForm::Rotate()
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
inline void QuaternionTransForm::SelfDefineAxisRotate()
{  
    float deltaRotation = slefDefineTargetRotate - slefDefineInitRotation;
    rotationQuat = glm::angleAxis(glm::radians(deltaRotation),slefDefineAxis);
}

inline void QuaternionTransForm::MouseMove(float xposIn,float yposIn)
{
    xposIn *= mouseSensitivity;
    yposIn *= mouseSensitivity;
    targetRotation.x += yposIn;
    targetRotation.y += xposIn;
}

inline void QuaternionTransForm::MouseScroll(float yoffset)
{
    //滑轮向前，视野变小，同样的显示区域内物体会变大
    fovY -= yoffset;
    if (fovY < 1.0f)
        fovY = 1.0f;
    if (fovY > 90.0f)
        fovY = 90.0f;
}

inline void QuaternionTransForm::KeyBoardMove(float delta)
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

inline void QuaternionTransForm::OnGUI()
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