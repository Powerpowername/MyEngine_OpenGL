#include "MyEngine.h"
#include "winConfig/winConfig.h"
#include <functional>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Model/model.hpp"
#include <chrono>
using namespace std::placeholders;
// #include <filesystem>
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
using WinConfig::Window;
int main()
{
    // std::cout<<std::filesystem::current_path();
#pragma region 窗口初始化
    // GLFWwindow* window = nullptr;
    Window window{4,5,"OenpGL Engine",SCR_WIDTH,SCR_HEIGHT};

    glfwMakeContextCurrent(window.Get());
    //根据窗口大小调整画布大小
    glfwSetFramebufferSizeCallback(window.Get(),[](GLFWwindow* window, int width, int height){
        glViewport(0, 0, width, height);
        Setting::pWindowSize.x = width;
        Setting::pWindowSize.y = height;
        // 同步更新相机宽高比
        if (Setting::MainCamera && Setting::MainCamera->transfrom)
        {
            Setting::MainCamera->transfrom->aspectRatio = (float)width / height;
        }
    });
    glfwSetCursorPosCallback(window.Get(),[](GLFWwindow* window, double xpos, double ypos){
        //仅在第一次调用静态对象时会被初始化，后续不会再调用初始化
        static bool firstMouse = true;
        static float lastX = SCR_WIDTH / 2.0f;
        static float lastY = SCR_HEIGHT / 2.0f;

        if(!Setting::MainCamera->ShowMouseMoveFlag())
        {
            firstMouse = true;
            return ;
        }
        
        float xposIn = static_cast<float>(xpos);
        float yposIn = static_cast<float>(ypos);
        if(firstMouse)
        {
            lastX = xposIn;
            lastY = yposIn;
            firstMouse = false;
        }
        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;

        lastX = xpos;
        lastY = ypos;

        Setting::MainCamera->transfrom->MouseMove(xoffset,yoffset);
    });
    glfwSetScrollCallback(window.Get(),[](GLFWwindow* window, double xoffset, double yoffset){
        Setting::MainCamera->transfrom->MouseScroll(yoffset);
    });
    QuaternionTransForm transfrom(QuaternionTransForm::MoveObject::CAMERA);

    // 初始化GLAD加载器（获取OpenGL函数指针）
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

    // 初始化ImGui
    IMGUI_CHECKVERSION();//坚持h文件与cpp文件的版本是否一致
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;//出去未使用的警告
    // 启用键盘导航
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;//暂不启用
    ImGui_ImplGlfw_InitForOpenGL(window.Get(),true);
    string glsl_version;
    window.GetGLSLVersion(glsl_version);
    ImGui_ImplOpenGL3_Init(glsl_version.c_str());
 



#pragma endregion

    //配置全局Setting
    Setting::window = window.Get();
    window.GetWindowSize(Setting::pWindowSize.x,Setting::pWindowSize.y);
    // QuaternionCamera camera(vec3(0,0,0));
    QuaternionCamera camera;
    Setting::MainCamera = &camera;

    auto start = std::chrono::system_clock::now();
    //model resource/object/backpack
    // Model piano("../../resource/object/backpack/backpack.obj");
    auto end = std::chrono::system_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout<<"elapse time: "<<(delta.count()/1000)<<"ms"<<std::endl;


    glEnable(GL_DEPTH_TEST);//开启深度测试
    // glDepthFunc(GL_LEQUAL);

    while (!glfwWindowShouldClose(window.Get()))
    {

        // 开始ImGui帧
        ImGui_ImplOpenGL3_NewFrame();//这是 ImGui 与 OpenGL 渲染后端交互的初始化步骤
        ImGui_ImplGlfw_NewFrame();//这是 ImGui 与 GLFW 窗口系统交互的初始化步骤
        ImGui::NewFrame();//新帧初始化，处理GLFW的输入
        float deltaTime = io.DeltaTime; // 每帧间隔时间单位：秒
        glClearColor(0.2,0.2,0.5,1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清屏（颜色缓冲 + 深度缓冲）

        Factory::ControlGui();
        Factory::OnGui();
        Factory::Render();
        Factory::Update();


        // 渲染ImGui
        ImGui::Render();//生成控件的渲染指令，与平台无关
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());//将上一步生成的渲染指令运行渲染

        glfwSwapBuffers(window.Get());
        glfwPollEvents();


    }
    // 清理ImGui资源
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    //终止GLFW
    glfwTerminate();
    return 0;
}