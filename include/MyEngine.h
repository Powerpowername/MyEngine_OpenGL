#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS // 必须在 #include "imgui.h" 之前
#define IMGUI_IMPL_OPENGL_LOADER_GLAD // 告诉 ImGui 使用 GLAD
#define GLM_ENABLE_EXPERIMENTAL 
#define GLEW_STATIC
#define var auto
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
using std::string;
using glm::vec3,glm::vec2,glm::mat3,glm::mat4,glm::quat;
// #pragma region 类名前向声明
// class Object;
// class QuaternionTransForm;//包含摄像机与普通绘制物体的四元数实现的旋转变化与位移变化
// class Input;
// class QuaternionCamera;
// class Setting;//管理窗口和全局其他对象
// #pragma endregion

#pragma region Object
class Object
{
public:
    //拷贝构造、拷贝赋值、析构函数均未显式定义，且未显式声明移动操作，才会自动生成移动构造函数
    string name;
    //function
    Object() = default;
    //纯虚析构函数必须要有定义，不然没法释放资源
    virtual ~Object();
    virtual void OnGUI() = 0;
    virtual void Update() = 0;
};
#pragma endregion

#pragma region QuaternionTransForm : public Object
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
    QuaternionTransForm(QuaternionTransForm::MoveObject moveObject,vec3 position = vec3(0,0,0),vec3 forward = vec3(0,0,-1),vec3 right = vec3(1,0,0),float fovY = 90.0f,float aspectRatio = 16.0f/9.0f,float nearZ = 0.1f,float farZ = 100.0f);
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

#pragma region Input
class Input
{
protected:
    //wsad依次代表低四位比特数据
    static unsigned char status;

    static void SetWordW(bool isPush);
    static void SetWordS(bool isPush);
    static void SetWordA(bool isPush);
    static void SetWordD(bool isPush);

    static float currentFrameTime;
    static float lastFrameTime;
    static bool firstMouse;//是否是鼠标首次进入窗口

public:
    // static GLFWwindow* window;
    static void GetKeyBoardInput();//处理按键处理事件

    static bool GetWordW();
    static bool GetWordS();
    static bool GetWordA();
    static bool GetWordD();
    static float DeltaFrameTime();//获取前后帧的时间差
    static void FreshFrameTime();//刷新前后帧的时间
    // static void ClearInputEveryFrame();//每帧重置status
};
#pragma endregion

#pragma region QuaternionCamera : public Object
class QuaternionCamera : public Object
{
protected:
    static int cameraNum;
    int ID = -1;
    bool enableMouseMove = false;
    void SetCameraPosition(vec3 position);
    void SetCameraFovY(float fovY);
    void SetCameraAspectRatio(float aspectRatio);
    void SetCameraNearZ(float nearZ);
    void SetCameraFarZ(float farZ);
public:
    std::unique_ptr<QuaternionTransForm> transfrom = nullptr;
    QuaternionCamera(vec3 position = vec3(0,0,3), vec3 forward = vec3(0,0,-1), vec3 right = vec3(-1,0,0), float fovY = 90.f, float aspectRatio = 16.0f/9.0f, float nearZ = 0.1f, float farZ = 100.0f);
    ~QuaternionCamera() override;
    void OnGUI() override;
    void Update() override;
    bool ShowMouseMoveFlag();
    mat4 GetViewMatrixTransposed();
    mat4 GetProjectionMatrixTransposed();
    vec3 GetCameraPosition();

    
    QuaternionCamera(const QuaternionCamera&) = delete;
    QuaternionCamera(QuaternionCamera&) = delete;
    QuaternionCamera& operator=(const QuaternionCamera&) = delete;
    //定义移动构造函数
    QuaternionCamera(QuaternionCamera&& other);
};

#pragma endregion

#pragma region RenderObject : public Object
class RenderObject : public Object
{
protected:
    unsigned int VBO = 0;
    unsigned int VAO = 0;
    unsigned int diffuseMap = 0;
    unsigned int normalMap = 0;
    unsigned int tangentMap = 0;
    unsigned int bittangentMap = 0;
    std::unique_ptr<QuaternionTransForm> transform = nullptr;

    // std::unique_ptr<float> vertices = nullptr;
public:
    ~RenderObject();
    virtual void SetShader(Shader &shader,Object* viewObject) = 0;
    virtual void RenderInit(unsigned int diffuseMap,unsigned int normalMap){};
    virtual void Render(Shader& shader){};
    RenderObject() = default;
    RenderObject(const RenderObject&) = delete;
    RenderObject(RenderObject&) = delete;
    RenderObject& operator=(const RenderObject&) = delete;
    //删除=的const版本编译器就不会生成非const版本的了
    // RenderObject& operator=(RenderObject&) = delete;
    //定义移动构造函数
    RenderObject(RenderObject&& other);
};

#pragma endregion


#pragma region Cube : public RenderObject
class Cube : public RenderObject
{
private:
    static int cubeNum;
    int cubeID = -1;
    float vertices[11 * 36] = {//24*8    
        // positions          // normals           // texture coords 
    // 后面: 切线(1,0,0)
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, 0.0f,

    // 前面: 切线(1,0,0)
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, 0.0f,

    // 左面: 切线(0,1,0)
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, 0.0f,

    // 右面: 切线(0,1,0)
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, 0.0f,

    // 下面: 切线(1,0,0)
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f, 1.0f, 0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, 0.0f,

    // 上面: 切线(1,0,0)
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f, 1.0f, 0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f, 0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, 0.0f
    };

public:
    
    Cube(vec3 position = vec3(0,0,0),vec3 forward = vec3(0,0,1),vec3 right = vec3(1,0,0));
    ~Cube();
    void RenderInit(unsigned int diffuseMap,unsigned int normalMap) override;
    void SetShader(Shader &shader,Object* viewObject) override;
    void Render(Shader& shader) override;
    void OnGUI() override;
    void Update() override;

    Cube(const Cube&) = delete;
    Cube(Cube&) = delete;
    Cube& operator=(const Cube&) = delete;
    Cube(Cube&& other);

};
#pragma endregion

#pragma region SkyBox : publicObject
class SkyBox : public Object
{
protected:
    static int SkyBoxNum;
    int SkyBoxID = -1;
    //立方体贴图是不需要和普通纹理一样添加纹理坐标的，只要在着色器中通过方向向量采样
    float vertices[24] = {
        // 位置坐标（x,y,z）
        -1.0f, -1.0f, -1.0f, // 0: 后左下
        1.0f, -1.0f, -1.0f,  // 1: 后右下
        1.0f, 1.0f, -1.0f,   // 2: 后右上
        -1.0f, 1.0f, -1.0f,  // 3: 后左上
        -1.0f, -1.0f, 1.0f,  // 4: 前左下
        1.0f, -1.0f, 1.0f,   // 5: 前右下
        1.0f, 1.0f, 1.0f,    // 6: 前右上
        -1.0f, 1.0f, 1.0f    // 7: 前左上
    };
    //EBO使用的索引数据
    unsigned int indices[36] = {
        // 背面（Z负方向）
        0, 1, 2, 0, 2, 3,
        // 右面（X正方向）
        1, 5, 6, 1, 6, 2,
        // 前面（Z正方向）
        5, 4, 7, 5, 7, 6,
        // 左面（X负方向）
        4, 0, 3, 4, 3, 7,
        // 底面（Y负方向）
        4, 5, 1, 4, 1, 0,
        // 顶面（Y正方向）
        3, 2, 6, 3, 6, 7};
    unsigned int VBO = 0;//顶点缓冲对象
    unsigned int VAO = 0;//顶点数组对象，存的是顶点的属性指针
    unsigned int IBO = 0;//索引缓冲对象
    unsigned int textureID = 0;//天空盒子的纹理
    std::unique_ptr<QuaternionTransForm> transfrom = nullptr;
    // std::vector<string> face;

public:

    SkyBox();
    
    ~SkyBox();
    

    void RenderInit(const std::vector<string>& faces);//加载天空盒子
    void SetShader(Shader& shader,Object* viewObject);
    void Render(Shader& shader);
    void OnGUI() override;
    void Update() override;
    
    SkyBox(const SkyBox&) = delete;
    SkyBox(SkyBox&) = delete; 
    SkyBox& operator=(const SkyBox&) = delete;
    SkyBox(SkyBox&& other);

    
    
};


#pragma endregion




#pragma region AbstractLight : public Object
class AbstractLight : public Object
{
protected:
    vec3 lightColor{0,0,0};
    // unsigned int depthMapFBO = 0;//深度帧缓冲
    // unsigned int depthMap = 0;//深度纹理附件
    // //纹理分辨率
    // unsigned int ShadowWidth = 1024;
    // unsigned int ShadowHeight = 1024;
    std::unique_ptr<QuaternionTransForm> transform = nullptr;

public:
    AbstractLight() = default;
    // void SetDepthResolution(unsigned int ShadowWidth,unsigned int ShadowHeight);//设置深度纹理的分辨率
    virtual ~AbstractLight();
    // virtual void ConfigShadow(unsigned int depthMapFBO,unsigned int depthMap) = 0;//配置深度缓冲帧与深度纹理附件
    virtual void SetShader(Shader& shader) = 0;//给正常绘制的shader的配置接口
    // virtual void SetDepthShader(Shader& depthShader) = 0;//专为绘制阴影shader的接口配置
    /// @brief 使用绘制深度纹理的shader和被绘制对象的绘制函数来绘制深度纹理
    /// @param depthShader 绘制深度纹理的shader
    /// @param objectRenderFunc 被绘制对象的绘制函数
    // virtual void RenderDepth(Shader& depthShader,std::function<void(Shader& shader)> objectRenderFunc){};
    // virtual void SetLightShader(Shader& shader) = 0;
    // QuaternionTransForm* GetTransform(){return transform.get();}
    AbstractLight(const AbstractLight&) = delete;
    AbstractLight(AbstractLight&) = delete;
    AbstractLight& operator=(AbstractLight&) = delete;
    //移动构造
    AbstractLight(AbstractLight&& other);

};
#pragma endregion

#pragma region DirctionLight : public AbstractLight 
class DirctionLight : public AbstractLight 
{
protected:
    static int dirctionLightNum;
    int dirctionLightID = -1;
    
public:
    DirctionLight(vec3 lightColor,vec3 position = vec3(0,0,0),vec3 forward = vec3(0,0,-1),vec3 right = vec3(-1,0,0));
    ~DirctionLight(){};

    // void ConfigShadow(unsigned int depthMapFBO,unsigned int depthMap) override;
    void SetShader(Shader& shader) override;
    // void SetDepthShader(Shader& depthShader) override;
    // void RenderDepth(Shader& depthShader,std::function<void(Shader& shader)>) override;
    void OnGUI() override;
    void Update() override;
    // void SetLightShader(Shader& shader) override;


    DirctionLight(const DirctionLight&) = delete;
    DirctionLight(DirctionLight&) = delete;
    DirctionLight& operator=(DirctionLight&) = delete;
    //移动构造
    DirctionLight(DirctionLight&& other);
    
};
#pragma endregion

#pragma region PointLight : public AbstractLight 
class PointLight : public AbstractLight 
{
protected:
    static int pointLightNum;
    int pointLightID = -1;
    //衰减参数
	float constant = 1.0f;
	float linear = 0.14;
	float quadratic = 0.07f;

public:
    PointLight(vec3 lightColor,vec3 position = vec3(0,0,0),vec3 forward = vec3(0,0,-1),vec3 right = vec3(-1,0,0));
    ~PointLight(){};

    void SetShader(Shader& shader) override;
    void OnGUI() override;
    void Update() override;

    PointLight(const PointLight& ) = delete;
    PointLight(PointLight&) = delete;
    PointLight& operator=(const PointLight&) = delete;
    PointLight(PointLight&& other);

};
#pragma endregion

#pragma region SpotLight : public AbstractLight 
class SpotLight : public AbstractLight
{
protected:
    static int spotLightNum;
    int spotLightID = -1;
    //衰减参数
	float constant = 1.0f;
	float linear = 0.14;
	float quadratic = 0.07f;
    //聚光范围
	float cosPhyInner = 12.5f;//内圈
	float cosPhyOuter= 20.0f;//外圈

public:
    SpotLight(vec3 lightColor,vec3 position = vec3(0,0,0),vec3 forward = vec3(0,0,-1),vec3 right = vec3(-1,0,0));
    ~SpotLight(){};

    void SetShader(Shader& shader) override;
    void OnGUI() override;
    void Update() override;

    SpotLight(const SpotLight& ) = delete;
    SpotLight(SpotLight&) = delete;
    SpotLight& operator=(const SpotLight&) = delete;
    SpotLight(SpotLight&& other);
};
#pragma endregion

#pragma region Setting
//尝试使用工厂模式设计
class Setting
{
public:
    static QuaternionCamera* MainCamera;
    static GLFWwindow* window;
    static glm::vec2 pWindowSize;//真正的窗口尺寸


};


class CameraFactory
{
protected:



public:


};
#pragma endregion



unsigned int loadTexture(const char*  path,bool reverse);


