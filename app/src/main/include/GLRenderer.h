//
// Created by DELL on 2025/10/17.
//

#ifndef MY_APPLICATION_EGL_RENDER_H
#define MY_APPLICATION_EGL_RENDER_H
#include <android/native_window.h>
#include <jni.h>
#include <android/log.h>
#define TAG "GLRenderer"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#include <string>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>   // 👈 关键！定义了 EGLImageKHR 和相关函数原型
#include <GLES3/gl32.h>
#include <GLES3/gl3ext.h>

#define GL_CALL(x) do{x;CheckGLError(__FILE__,__LINE__);}while(0)

#define LOG_THREAD_ID(msg) LOGI("%s: Thread ID: %lu", msg, (long unsigned int)pthread_self())

namespace egl{

    bool InitPrimaryRenderer(ANativeWindow* window);

    bool InitSecondaryRenderer(ANativeWindow* window);

    long long get_nano_time();

    void CheckGLError(const char* file,int line);

    void RenderWithUnityTexture();

    void setUnityPointer(int* uPara);



    extern const char* VERTEX_SHADER_SOURCE;

    extern const char* FRAGMENT_SHADER_SOURCE;
    extern EGLContext unity_context;
    extern EGLDisplay unityDisplay;
    extern EGLDisplay shareDisplay;
    extern EGLConfig unityConfig;
    extern GLuint texture11;
    extern EGLImageKHR eglImage;
    extern int* unityPara;
    extern EGLContext second_context;
    extern EGLSurface unitySurface;
    extern int unityImg[3];
    extern GLuint localTex;
    extern JNIEnv* kt;
    extern bool paused;

    class GLRenderer {

    protected:
        EGLContext m_context;
        //EGLDisplay unityDisplay;
        ANativeWindow* m_window = nullptr;
        EGLSurface m_surface= EGL_NO_SURFACE;
        int32_t m_width,m_height;

        GLuint m_program;
        GLuint m_vao;

        virtual bool SetupEGL(EGLConfig* config);
        virtual void Rendering() = 0;


    public:

        bool Initialization(ANativeWindow* window);

    };
}


#endif //MY_APPLICATION_EGL_RENDER_H
