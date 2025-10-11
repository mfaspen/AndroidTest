//
// Created by DELL on 2025/10/10.
//

#ifndef MY_APPLICATION_NATIVE_RENDER_H
#define MY_APPLICATION_NATIVE_RENDER_H

#include <android/native_window.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <string>

class NativeRenderer {
public:
    NativeRenderer();
    ~NativeRenderer();

    // 1. 初始化 EGL 环境并创建 GL 资源
    bool init(ANativeWindow* window);

    // 2. 渲染一帧
    void render();

    // 3. 销毁 EGL 环境和 GL 资源
    void destroy();

private:
    ANativeWindow* m_window = nullptr;
    EGLDisplay m_display = EGL_NO_DISPLAY;
    EGLSurface m_surface = EGL_NO_SURFACE;
    EGLContext m_context = EGL_NO_CONTEXT;

    // GL 资源
    GLuint m_program = 0;
    GLuint m_vao = 0;

    // 内部函数
    bool setupEGL(EGLConfig* config);
    bool setupGL();

    // 编译着色器
    GLuint compileShader(GLenum type, const std::string& source);
    // 链接程序
    GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader);
};



#endif //MY_APPLICATION_NATIVE_RENDER_H
