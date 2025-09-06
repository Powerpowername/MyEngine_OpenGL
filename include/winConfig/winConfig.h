// #include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <iostream>
#include <string>
using std::string;
namespace WinConfig
{
class Window;
}

class WinConfig::Window
{
protected:
    GLFWwindow* window = nullptr;
    string glsl_version;
    float width = 0;
    float height = 0;
public:
    Window(unsigned int major,unsigned int minor,string windowName,float width,float height);
    ~Window();
    GLFWwindow* Get();
    void GetWindowSize(float& width,float& height);
    void GetGLSLVersion(string& glsl_version);
    void SetCurrentWindow();
    Window(const Window&) = delete;
    Window(Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&& other);
};
inline WinConfig::Window::Window(unsigned int major, unsigned int minor, string windowName,float width,float height)
{
    if(!glfwInit())
    {
        std::cerr << "glfwInit failed" <<std::endl;
        return ;
    }
    glsl_version = "#version " + std::to_string(major) + std::to_string(minor) + std::to_string(0) + " core";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    
    this->width = width;
    this->height = height;

    window = glfwCreateWindow(width,height,windowName.c_str(),nullptr,nullptr);
    if(window == nullptr)
    {
        std::cerr << "无法创建GLFW窗口" << std::endl;
        glfwTerminate();
        return ;
    }
}
inline WinConfig::Window::~Window()
{
    if(window)
        glfwDestroyWindow(window);
}
inline GLFWwindow *WinConfig::Window::Get()
{
    return window;
}

inline void WinConfig::Window::GetWindowSize(float& width, float& height)
{
    width = this->width;
    height = this->height;
}

inline void WinConfig::Window::GetGLSLVersion(string& glsl_version)
{
    glsl_version =this->glsl_version;
}

inline void WinConfig::Window::SetCurrentWindow()
{
    glfwMakeContextCurrent(window);
    //启用垂直同步，避免显卡渲染与屏显刷新率不匹配导致的画面撕裂
    glfwSwapInterval(1);
}

inline WinConfig::Window::Window(Window && other):window(other.window)
{
    other.window = nullptr;
    this->glsl_version = other.glsl_version;
    this->width = width;
    this->height = height;
}
