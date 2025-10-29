//
// Created by DELL on 2025/10/17.
//

#ifndef MY_APPLICATION_EGL_RENDER_H
#define MY_APPLICATION_EGL_RENDER_H
#include <android/native_window.h>
#include <android/native_window_jni.h> // Contains ANativeWindow_fromSurface
#include <jni.h>
#include <android/log.h>
#include <vector>
#define TAG "GLRenderer"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#include <string>


#include <unistd.h>
#include <EGL/egl.h>
#include <GLES3/gl32.h>
#include <GLES3/gl3ext.h>
#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT 0x80E1
#endif
#ifndef EGL_CONTEXT_OPENGL_SHARE_CONTEXT_KHR
#define EGL_CONTEXT_OPENGL_SHARE_CONTEXT_KHR 0x8000 // 这个值需要根据实际文档确认
#endif
#define GL_CALL(x) (x)//do{x;CheckGLError(__FILE__,__LINE__);}while(0)

#define LOG_THREAD_ID(msg) LOGI("%s: Thread ID: %lu", msg, (long unsigned int)pthread_self())

namespace egl{

    bool InitPrimaryRenderer(ANativeWindow* window);

    bool InitSecondaryRenderer(ANativeWindow* window);
    long long get_nano_time();

    void SurfaceDestroyed();

    void CheckGLError(const char* file,int line);
    void CreateTexture11();
    void getConfig();

    extern const char* VERTEX_SHADER_SOURCE;

    extern const char* FRAGMENT_SHADER_SOURCE;

    extern EGLContext unity_context;
    extern EGLContext primaryContext;
    extern EGLContext second_context;

    extern EGLDisplay unityDisplay;
    //extern EGLDisplay unityDisplay;

    extern EGLSurface unitySurface;

    extern EGLConfig unityConfig;

    extern GLuint texture11;
    extern GLuint localTex;

    extern int* unityPara;
    extern int unityImg[3];

    extern JNIEnv* kt;

    extern bool paused;


    class GLRenderer {

    public:
        ANativeWindow* m_window = nullptr;
        EGLSurface m_surface= EGL_NO_SURFACE;
        int32_t m_width,m_height;
        //EGLDisplay unityDisplay;
        GLuint m_program;
        GLuint m_vao;

        virtual bool SetupEGL(EGLConfig* config);
        virtual void Rendering() = 0;


    public:

        bool Initialization(ANativeWindow* window);

    };
}


#endif //MY_APPLICATION_EGL_RENDER_H
