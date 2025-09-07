#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS // 必须在 #include "imgui.h" 之前
#define IMGUI_IMPL_OPENGL_LOADER_GLAD // 告诉 ImGui 使用 GLAD
#define GLM_ENABLE_EXPERIMENTAL 
#define GLEW_STATIC
#define var auto
#include "QuaternionTransForm.hpp"
#include "Model/model.hpp"
// using std::string,std::unique_ptr,std::make_unique;
// using glm::vec3,glm::vec2,glm::mat3,glm::mat4,glm::quat;
#pragma region 类名前向声明
class Input;
class QuaternionCamera;
class Setting;//管理窗口和全局其他对象


class CubeFactory;
class LightFactory;
class DirctionLightFactory;
class PointLightFactory;
class SpotLightFactory;
class SkyBoxFactory;
class ModelFactory;
class Cube;
class DirctionLight;


class PointLight;
class SpotLight;
class RenderObject;
class Factory;//工厂类，管理所有工厂
unsigned int loadTexture(const char*  path,bool reverse);
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
    virtual void SetShader(Object* viewObject) = 0;
    virtual void RenderInit(unsigned int diffuseMap,unsigned int normalMap){};
    virtual void Render(){};
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
    Shader shader;
public:
    
    Cube(vec3 position = vec3(0,0,0),vec3 forward = vec3(0,0,1),vec3 right = vec3(1,0,0));
    ~Cube();
    void RenderInit(unsigned int diffuseMap,unsigned int normalMap) override;
    Shader& GetShader(){return shader;};
    void SetShader(Object* viewObject) override;
    void Render() override;
    void OnGUI() override;
    void Update() override;

    Cube(const Cube&) = delete;
    Cube(Cube&) = delete;
    Cube& operator=(const Cube&) = delete;
    Cube(Cube&& other);

};
#pragma endregion

#pragma region SkyBox : public Object
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
    Shader shader;
public:

    SkyBox();
    
    ~SkyBox();
    

    void RenderInit(const std::vector<string>& faces);//加载天空盒子
    void SetShader(Object* viewObject);
    Shader& GetShader(){return shader;};
    void Render();
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
    virtual void SetShader(Shader& shader ,bool selectLight) = 0;//给正常绘制的shader的配置接口
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
    DirctionLight(vec3 lightColor = vec3(1,1,1),vec3 position = vec3(0,0,0),vec3 forward = vec3(0,0,-1),vec3 right = vec3(-1,0,0));
    ~DirctionLight(){dirctionLightNum--;};

    // void ConfigShadow(unsigned int depthMapFBO,unsigned int depthMap) override;
    void SetShader(Shader& shader ,bool selectLight = 1) override;
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
    PointLight(vec3 lightColor = vec3(1,1,1),vec3 position = vec3(0,0,0),vec3 forward = vec3(0,0,-1),vec3 right = vec3(-1,0,0));
    ~PointLight(){pointLightNum--;};

    void SetShader(Shader& shader,bool selectLight = 1) override;
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
    SpotLight(vec3 lightColor = vec3(1,1,1),vec3 position = vec3(0,0,0),vec3 forward = vec3(0,0,-1),vec3 right = vec3(-1,0,0));
    ~SpotLight() 
    { 
        spotLightNum--; 
    };

    void SetShader(Shader& shader,bool selectLight = 1) override;
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
    static vector<unique_ptr<QuaternionCamera>> cameras;
    static vector<unique_ptr<SkyBox>> skyBoxs;
    static vector<unique_ptr<Cube>> cubes;
    static vector<unique_ptr<Model>> models;
    static vector<unique_ptr<DirctionLight>> dirctionLights;
    static vector<unique_ptr<PointLight>> pointLights;
    static vector<unique_ptr<SpotLight>> spotLights;
    // friend Factory;
};

class CameraFactory
{
protected:

public:
    static unique_ptr<QuaternionCamera> CreateObject()
    {
        return make_unique<QuaternionCamera>();
    }

};

class SkyFactory
{
protected:

public:
    static unique_ptr<SkyBox> CreateObject()
    {
        return make_unique<SkyBox>();
    }
};

class CubeFactory
{
protected:

public:
    static unique_ptr<Cube> CreateObject()
    {
        return make_unique<Cube>();
    }
};

class ModeFactory
{
protected:

public:
    static unique_ptr<Model> CreateObject(const string& path)
    {
        return make_unique<Model>(path);
    }
};


class DirctionLightFactory
{
protected:

public:
    static unique_ptr<DirctionLight> CreateObject()
    {
        return make_unique<DirctionLight>();
    }
};

class PointLightFactory
{
protected:

public:
    static unique_ptr<PointLight> CreateObject()
    {
        return make_unique<PointLight>();
    }
};

class SpotLightFactory
{
protected:

public:
    static unique_ptr<SpotLight> CreateObject()
    {
        return make_unique<SpotLight>();
    }
};

class Factory
{
protected:
    static void createCamera()
    {
        Setting::cameras.push_back(CameraFactory::CreateObject());
    }
    static void deleteCamera()
    {
        Setting::cameras.pop_back();
    }
    static void createSkyBox()
    {
        // Setting::skyBoxs.push_back(SkyFactory::CreateObject());
        if(ImGui::BeginPopupModal("Please Input SkyBox Resource Path",NULL,ImGuiWindowFlags_AlwaysAutoResize))
        {
            std::vector<string> faces(6);
            char rightPath[256]{"../../resource/skyBox\\right.jpg"};
            ImGui::InputText("Right Face Path",rightPath,IM_ARRAYSIZE(rightPath));
            faces[0] = rightPath;
            char leftPath[256]{"../../resource/skyBox\\left.jpg"};
            ImGui::InputText("Left Face Path",leftPath,IM_ARRAYSIZE(leftPath));
            faces[1] = leftPath;
            char topPath[256]{"../../resource/skyBox\\top.jpg"};
            ImGui::InputText("Top Face Path",topPath,IM_ARRAYSIZE(topPath));
            faces[2] = topPath;
            char bottomPath[256]{"../../resource/skyBox\\bottom.jpg"};
            ImGui::InputText("Bottom Face Path",bottomPath,IM_ARRAYSIZE(bottomPath));
            faces[3] = bottomPath;
            char frontPath[256]{"../../resource/skyBox\\front.jpg"};
            ImGui::InputText("Front Face Path",frontPath,IM_ARRAYSIZE(frontPath));
            faces[4] = frontPath;
            char backPath[256]{"../../resource/skyBox\\back.jpg"};
            ImGui::InputText("Back Face Path",backPath,IM_ARRAYSIZE(backPath));
            faces[5] = backPath;

            if (ImGui::Button("Add"))
            {
                Setting::skyBoxs.push_back(SkyFactory::CreateObject());
                Setting::skyBoxs.back()->RenderInit(faces);
                ImGui::CloseCurrentPopup();
            }
            // 取消按钮：仅关闭弹窗，不创建任何内容
            ImGui::SameLine();
            if (ImGui::Button("Cancel"))
            {
                Setting::skyBoxs.pop_back();
                ImGui::CloseCurrentPopup(); // 取消也关闭弹窗
            }
            ImGui::EndPopup();
        }

    }
    static void deleteSkyBox()
    {
        Setting::skyBoxs.pop_back();
    }
    static void createCube()
    {
        //打开模态弹窗
        if(ImGui::BeginPopupModal("Please Input Cube Resource Path",NULL,ImGuiWindowFlags_AlwaysAutoResize))
        {
            unsigned int diffuseMap;
            unsigned int normalMap; 
            char diffusePath[256]{"E:\\LearnOpenGL-master\\resources\\textures\\bricks2.jpg"};
            ImGui::InputText("Cube Resource diffusePath",diffusePath,IM_ARRAYSIZE(diffusePath));
            char normalPath[256]{"E:\\LearnOpenGL-master\\resources\\textures\\bricks2_normal.jpg"};
            ImGui::InputText("Cube Resource normalPath",normalPath,IM_ARRAYSIZE(normalPath));

            if (ImGui::Button("Add"))
            {
                diffuseMap = loadTexture(diffusePath, false);
                normalMap = loadTexture(normalPath, false);
                // std::cout << "diffuseMap ID: " << diffuseMap << std::endl;
                // std::cout << "normalMap ID: " << normalMap << std::endl;
                Setting::cubes.push_back(CubeFactory::CreateObject());
                Setting::cubes.back()->RenderInit(diffuseMap, normalMap);
                ImGui::CloseCurrentPopup();
            }
            // 取消按钮：仅关闭弹窗，不创建任何内容
            ImGui::SameLine();
            if (ImGui::Button("Cancel"))
            {
                ImGui::CloseCurrentPopup(); // 取消也关闭弹窗
            }
            ImGui::EndPopup();
        }

    }
    static void deleteCube()
    {
        Setting::cubes.pop_back();
    }
    static void createModel()
    {
        static char path[256] = "../../resource/object/backpack/backpack.obj";
        if(ImGui::BeginPopupModal("Please Input Model Resource Path",NULL,ImGuiWindowFlags_AlwaysAutoResize))//自动调整大小
        {
            ImGui::InputText("Model Resource Path",path,IM_ARRAYSIZE(path));
            if(ImGui::Button("Add"))
            {
                Setting::models.push_back(ModeFactory::CreateObject(string(path)));
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if(ImGui::Button("Cancel"))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
        // Setting::models.push_back(ModeFactory::CreateObject(path));
    }
    static void deleteModel()
    {
        Setting::models.pop_back();
    }
    static void createDirctionLight()
    {
        Setting::dirctionLights.push_back(DirctionLightFactory::CreateObject());
    }
    static void deleteDirctionLight()
    {
        Setting::dirctionLights.pop_back();
    }
    static void createPointLight()
    {
        Setting::pointLights.push_back(PointLightFactory::CreateObject());
    }
    static void deletePointLight()
    {
        Setting::pointLights.pop_back();
    }
    static void createSpotLight()
    {
        Setting::spotLights.push_back(SpotLightFactory::CreateObject());
    }
    static void deleteSpotLight()
    {
        Setting::spotLights.pop_back();
    }
public:
#pragma region 添加与删除对象
    static  void ControlGui()
    {
        ImGui::Begin("Control Panel");
        //Camera
        ImGui::Separator();
        if(ImGui::Button("Add Camera"))
        {
            createCamera();
        }
        ImGui::SameLine();
        if(ImGui::Button("Delete Camera"))
        {
            if(Setting::cameras.size() != 0)
                deleteCamera();

        }
        ImGui::Text("Current Camera Num: %d",Setting::cameras.size());

        //SkyBox
        ImGui::Separator();
        if(ImGui::Button("Add SkyBox"))
        {
            //打开模态弹窗
            ImGui::OpenPopup("Please Input SkyBox Resource Path");
        }
        createSkyBox();
        ImGui::SameLine();
        if(ImGui::Button("Delete SkyBox"))
        {
            if(Setting::skyBoxs.size() != 0)
                deleteSkyBox();

        }
        ImGui::Text("Current SkyBox Num: %d",Setting::skyBoxs.size());

        //Cube
        ImGui::Separator();
        if(ImGui::Button("Add Cube"))
        {
            //打开模态弹窗
            ImGui::OpenPopup("Please Input Cube Resource Path");
        }
        createCube();
        
        ImGui::SameLine();
        if(ImGui::Button("Delete Cube"))
        {
            if(Setting::cubes.size() != 0)
                deleteCube();
        }
        ImGui::Text("Current Cube Num: %d",Setting::cubes.size());

        //DirctionLight
        ImGui::Separator();
        if(ImGui::Button("Add DirctionLight"))
        {
            createDirctionLight();
        }
        ImGui::SameLine();
        if(ImGui::Button("Delete DirctionLight"))
        {
            if(Setting::dirctionLights.size() != 0)
                deleteDirctionLight();
        }
        ImGui::Text("Current DirctionLight Num: %d",Setting::dirctionLights.size());

        //PointLight
        ImGui::Separator();
        if(ImGui::Button("Add PointLight"))
        {
            createPointLight();
        }
        ImGui::SameLine();
        if(ImGui::Button("Delete PointLight"))
        {
            if(Setting::pointLights.size() != 0)
                deletePointLight();
        }
        ImGui::Text("Current PointLight Num: %d",Setting::pointLights.size());

        //SpotLight
        ImGui::Separator();
        if(ImGui::Button("Add SpotLight"))
        {
            createSpotLight();
        }
        ImGui::SameLine();
        if(ImGui::Button("Delete SpotLight"))
        {
            if(Setting::spotLights.size() != 0)
                deleteSpotLight();
        }
        ImGui::Text("Current SpotLight Num: %d",Setting::spotLights.size());

        //Model
        ImGui::Separator();
        if(ImGui::Button("Add Model"))
        {
            //打开模态弹窗
            ImGui::OpenPopup("Please Input Model Resource Path");
            // //打开模态弹窗
            // if(ImGui::BeginPopupModal("Please Input Model Resource Path",NULL,ImGuiWindowFlags_AlwaysAutoResize))//自动调整大小
            // {
            //     char modelPath[256];
            //     ImGui::InputText("Model Resource Path",modelPath,IM_ARRAYSIZE(modelPath));
            //     if(ImGui::Button("Add"))
            //     {
            //         createModel(string(modelPath));
            //         ImGui::CloseCurrentPopup();
            //     }
            //     ImGui::SameLine();
            //     if(ImGui::Button("Cancel"))
            //         ImGui::CloseCurrentPopup();
            //     ImGui::EndPopup();
            // }
        }
        createModel();
        ImGui::SameLine();
        if(ImGui::Button("Delete Model"))
        {
            if(Setting::models.size() != 0)
                deleteModel();
        }
        ImGui::Text("Current Model Num: %d",Setting::models.size());

        ImGui::End();
    }

#pragma endregion



    
    static void Render()
    {
        //配置绘制对象
        for(auto& it : Setting::cubes)
        {
            //设置光源
            for(auto& dirLight : Setting::dirctionLights)
                dirLight->SetShader(it->GetShader());
            for(auto& pointLight : Setting::pointLights)
                pointLight->SetShader(it->GetShader());
            for(auto& spotLight : Setting::spotLights)
                spotLight->SetShader(it->GetShader());
            // std::cout<<"dfa"<<std::endl;
            //设置MVP
            it->SetShader(Setting::MainCamera->transfrom.get());
            //渲染
            it->Render();
        }



        
        for(auto& it : Setting::skyBoxs)
        {
            //设置MVP
            it->SetShader(Setting::MainCamera->transfrom.get());
            //渲染
            it->Render();
        }

        for(auto& it : Setting::models)
        {
            //设置MVP
            it->SetShader(Setting::MainCamera->transfrom.get());
            //渲染
            it->Render();
        }
        



        //重置绘制对象的光源状态
        for(auto& it : Setting::cubes)
        {
            //设置光源
            for(auto& dirLight : Setting::dirctionLights)
                dirLight->SetShader(it->GetShader(),0);
            for(auto& pointLight : Setting::pointLights)
                pointLight->SetShader(it->GetShader(),0);
            for(auto& spotLight : Setting::spotLights)
                spotLight->SetShader(it->GetShader(),0);
            // // std::cout<<"dfa"<<std::endl;
            // //设置MVP
            // it->SetShader(Setting::MainCamera->transfrom.get());
            // //渲染
            // it->Render();
        }

    }





    static void OnGui()
    {
        ImGui::Begin("Obejct Gui");
        for(auto& camera : Setting::cameras)
            camera->OnGUI();
        for(auto& skyBox : Setting::skyBoxs)
            skyBox->OnGUI();
        for(auto& cube : Setting::cubes)
            cube->OnGUI();
        for(auto& model : Setting::models)
            model->OnGUI();
        for(auto& dirLight : Setting::dirctionLights)
            dirLight->OnGUI();
        for(auto& pointLight : Setting::pointLights)
            pointLight->OnGUI();
        for(auto& spotLight : Setting::spotLights)
            spotLight->OnGUI();
        ImGui::End();
    }
    static void Update()
    {
        if(Setting::cameras.empty() != true )
            Setting::MainCamera = Setting::cameras.front().get();
        for(auto& camera : Setting::cameras)
            camera->Update();
        for(auto& skyBox : Setting::skyBoxs)
            skyBox->Update();
        for(auto& cube : Setting::cubes)
            cube->Update();
        for(auto& model : Setting::models)
            model->Update();
        for(auto& dirLight : Setting::dirctionLights)
            dirLight->Update();
        for(auto& pointLight : Setting::pointLights)
            pointLight->Update();
        for(auto& spotLight : Setting::spotLights)
            spotLight->Update();
    }




};



#pragma endregion




