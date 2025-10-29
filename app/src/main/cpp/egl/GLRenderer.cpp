//
// Created by DELL on 2025/10/17.
//

#include <unistd.h>
#include "GLRenderer.h"
#include "PrimaryRenderer.h"
#include "SecondaryRenderer.h"

namespace egl{

    const char* VERTEX_SHADER_SOURCE = R"(#version 300 es
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 coordinate;
out vec2 V_Texcoord;
void main() {
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
)";

    const char* FRAGMENT_SHADER_SOURCE = R"(#version 300 es
precision mediump float;
uniform sampler2D U_Texture;
in vec2 V_Texcoord;

out vec4 FragColor;

void main() {

    FragColor = texture(U_Texture,V_Texcoord);
    FragColor.a = 1.0;
}
)";

    long long get_nano_time(){

        using namespace std::chrono;
        auto now = system_clock::now();
        return duration_cast<milliseconds>(now.time_since_epoch()).count();
    }

    void CheckGLError(const char* file,int line){
        GLenum error = glGetError();
        if(error!= GL_NO_ERROR){
            switch(error){
                case GL_INVALID_ENUM:
                    LOGE("GL_ERROR: GL_INVALID_ENUM %s :%d",file,line);
                    break;
                case GL_INVALID_VALUE:
                    LOGE("GL_ERROR: GL_INVALID_VALUE %s :%d",file,line);
                    break;
                case GL_INVALID_INDEX:
                    LOGE("GL_ERROR: GL_INVALID_INDEX %s :%d",file,line);
                    break;
                case GL_OUT_OF_MEMORY:
                    LOGE("GL_ERROR: GL_OUT_OF_MEMORY %s :%d",file,line);
                    break;
                default:
                    LOGE("GL_ERROR: default 0X%x %s :%d",error,file,line);
                    break;
            }
        }
    }

    PrimaryRenderer primaryRenderer;
    SecondaryRenderer secondaryRenderer;


    EGLContext unity_context = EGL_NO_CONTEXT;

    EGLContext second_context = EGL_NO_CONTEXT;
    EGLContext primaryContext = EGL_NO_CONTEXT;

    EGLDisplay unityDisplay = EGL_NO_DISPLAY;
    //EGLDisplay unityDisplay = EGL_NO_DISPLAY;

    EGLConfig unityConfig = nullptr;

    EGLSurface unitySurface;

    bool InitPrimaryRenderer(ANativeWindow* window){
        return primaryRenderer.Initialization(window);
    }

    bool InitSecondaryRenderer(ANativeWindow* window) {
        while (primaryContext == EGL_NO_CONTEXT){
            usleep(16000);
        }
        return secondaryRenderer.Initialization(window);

    }
    void SurfaceDestroyed(){

        if (primaryRenderer.m_surface != EGL_NO_SURFACE) {
            eglDestroySurface(unityDisplay, primaryRenderer.m_surface);
            primaryRenderer.m_surface = EGL_NO_SURFACE;
        }
        if (primaryRenderer.m_window) {
            ANativeWindow_release(primaryRenderer.m_window);
            primaryRenderer.m_window = nullptr;
        }
    }

    extern GLuint texture11 = 0;
    /*
        * 创建单色纹理
        */
    void CreateTexture11(){

        glGenTextures(1, &texture11);
        glBindTexture(GL_TEXTURE_2D, texture11);

// 避免 UNPACK 对齐问题（1x1）
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

// 纯红色（R,G,B,A）
        unsigned char pixel[4] = { 255, 0, 0, 255 }; // 红色不透明

// 上传 1x1 纹理
        glTexImage2D(GL_TEXTURE_2D,
                     0,                // level
                     GL_RGBA,          // internal format (GLES2 用 GL_RGBA)
                     1, 1,             // width, height
                     0,                // border
                     GL_RGBA,          // format
                     GL_UNSIGNED_BYTE, // type
                     pixel);

// 纹理参数
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

// 解绑（可选）
        glBindTexture(GL_TEXTURE_2D, 0);
    }


    EGLConfig config;
    EGLConfig shareConfig = nullptr;




    void getConfig(){

        EGLint configID = 0;
        unitySurface = eglGetCurrentSurface(EGL_DRAW);

        eglQuerySurface(unityDisplay, unitySurface, EGL_CONFIG_ID, &configID);
        //eglQueryContext(shareDisplay, unity_context, EGL_CONFIG_ID, &configID);

        EGLint numConfigs = 0;
        eglGetConfigs(unityDisplay, nullptr, 0, &numConfigs);

        std::vector<EGLConfig> configs(numConfigs);
        eglGetConfigs(unityDisplay, configs.data(), numConfigs, &numConfigs);

        for (int i = 0; i < numConfigs; ++i) {
            EGLint id;
            eglGetConfigAttrib(unityDisplay, configs[i], EGL_CONFIG_ID, &id);
            LOGE("looking id:%d   numConfigs:%d",id,configID);
            if (id == configID) {
                unityConfig = configs[i];
                LOGE("looking id unity config get success!");
                break;
            }
        }
    }



    bool GLRenderer::Initialization(ANativeWindow *window) {
        m_window = window;

        if (!m_window) {
            LOGE("ANativeWindow is NULL.");
            return false;
        }
        if(shareConfig == nullptr){

            if (!SetupEGL(&shareConfig)) {
                LOGE("EGL setup failed.");
                return false;
            }
        }else{

            if (!SetupEGL(&config)) {
                LOGE("EGL setup failed.");
                return false;
            }
        }
        if(unityDisplay == EGL_NO_DISPLAY){
            // 1. 获取 Display
            unityDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
            if (unityDisplay == EGL_NO_DISPLAY) {
                LOGE("eglGetDisplay failed: %x", eglGetError());
                return false;
            }
        }
        //if(m_context == EGL_NO_CONTEXT){
        // 5. 创建 EGL Context (请求 ES 3.0)
        if(primaryContext == EGL_NO_CONTEXT){


            EGLint ctxAttribs[] = {EGL_CONTEXT_CLIENT_VERSION,3, EGL_NONE};

            EGLint surfaceAttribs[] = {
                    EGL_RENDER_BUFFER,
                    EGL_SINGLE_BUFFER,
                    EGL_NONE // 必须以 EGL_NONE 结束
            };

            while (unity_context == EGL_NO_CONTEXT){
                usleep(16000);
            }

            primaryContext = eglCreateContext(unityDisplay, config, unity_context, ctxAttribs);

            if (primaryContext == EGL_NO_CONTEXT) {
                LOGE("eglCreateContext failed: %x", eglGetError());
                return false;
            }

            if (m_surface != EGL_NO_SURFACE) {
                eglMakeCurrent(unityDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
                eglDestroySurface(unityDisplay, m_surface);
                m_surface = EGL_NO_SURFACE;
            }

            LOGE("ANativeWindow is width:%d unityDisplay: %p ", ANativeWindow_getWidth(m_window), unityDisplay);
            // 4. 创建 EGL Surface
            m_surface = eglCreateWindowSurface(unityDisplay, config, m_window, surfaceAttribs);
            if (m_surface == EGL_NO_SURFACE) {
                LOGE("eglCreateWindowSurface 111 failed: %x", eglGetError());
                return false;
            }
            //}

            // 6. 绑定上下文
            if (eglMakeCurrent(unityDisplay, m_surface, m_surface, primaryContext) == EGL_FALSE) {
                LOGE("eglMakeCurrent failed: %x", eglGetError());
                return false;
            }
//            while(second_context==EGL_NO_CONTEXT){
//                usleep(16000);
//            }
            CreateTexture11();
            // 获取视口大小
            EGLint width, height;
            eglQuerySurface(unityDisplay, m_surface, EGL_WIDTH, &width);
            eglQuerySurface(unityDisplay, m_surface, EGL_HEIGHT, &height);
            m_width = width;
            m_height = height;

            LOGI("EGL context made current successfully!");


        }else{

            EGLint surfaceAttribs[] = {
                    EGL_WIDTH, 1920,
                    EGL_HEIGHT, 1080,
                    EGL_NONE
            };
            const char *extensions = eglQueryString(unityDisplay, EGL_EXTENSIONS);
            if (strstr(extensions, "EGL_KHR_create_context") == NULL) {
                // 错误：系统不支持创建上下文的KHR扩展
                // 此时，你可能需要查找该平台特有的共享上下文方法
                return false;
            }
            EGLint ctxAttribs[] = {EGL_CONTEXT_CLIENT_VERSION,3,
                                   // 核心：指定共享的上下文 (使用 KHR 扩展的键)
            //EGL_CONTEXT_OPENGL_SHARE_CONTEXT_KHR, (EGLint)primaryContext,
            EGL_NONE};
            // 4. 创建 EGL Surface
            m_surface = eglCreatePbufferSurface(unityDisplay, config, surfaceAttribs);
            if (m_surface == EGL_NO_SURFACE) {
                LOGE("eglCreateWindowSurface second failed: %x", eglGetError());
                return false;
            }
            LOGI("EGL second_context made current successfully!");
            second_context = eglCreateContext(unityDisplay, config, primaryContext, ctxAttribs);

            if (second_context == EGL_NO_CONTEXT) {
                LOGE("eglCreateContext second_context failed: %x", eglGetError());
                return false;
            }
            //}

            // 6. 绑定上下文
            if (eglMakeCurrent(unityDisplay, m_surface, m_surface, second_context) == EGL_FALSE) {
                LOGE("eglMakeCurrent failed: %x", eglGetError());
                return false;
            }

            // 获取视口大小
            EGLint width, height;
            eglQuerySurface(unityDisplay, m_surface, EGL_WIDTH, &width);
            eglQuerySurface(unityDisplay, m_surface, EGL_HEIGHT, &height);
            m_width = width;
            m_height = height;
            LOGI("EGL second_context made current successfully!");



        }

        // 7. 设置 GL 资源
        Rendering();


        return true;
    }

    bool GLRenderer::SetupEGL(EGLConfig* config) {
        if(unityDisplay == EGL_NO_DISPLAY){
            // 1. 获取 Display
            unityDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
            if (unityDisplay == EGL_NO_DISPLAY) {
                LOGE("eglGetDisplay failed: %x", eglGetError());
                return false;
            }
        }

        // 2. 初始化 EGL
        EGLint major, minor;
        if (eglInitialize(unityDisplay, &major, &minor) == EGL_FALSE) {
            LOGE("eglInitialize failed: %x", eglGetError());
            return false;
        }
        LOGI("EGL initialized. Version %d.%d", (int)major, (int)minor);

        // 3. 配置 EGL 属性 (请求 ES 3.0)
        EGLint attribs[] = {
                EGL_RENDERABLE_TYPE,
                EGL_OPENGL_ES2_BIT, // 关键：请求 ES 3.0
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_BLUE_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_RED_SIZE, 8,
                EGL_DEPTH_SIZE, 16,
                EGL_NONE
        };
        EGLint configAttribs[] = {
                EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,             // 只要 PBUFFER，不要 OR
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,   // Unity 是 ES3
                EGL_RED_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_ALPHA_SIZE, 8,
                EGL_DEPTH_SIZE, 24,
                EGL_NONE
        };
        if(primaryContext == EGL_NO_CONTEXT){

            EGLint numConfigs;
            if (eglChooseConfig(unityDisplay, attribs, config, 1, &numConfigs) == EGL_FALSE || numConfigs == 0) {
                LOGE("eglChooseConfig failed or found no configs: %x", eglGetError());
                return false;
            }
        }else{

            EGLint numConfigs;
            if (eglChooseConfig(unityDisplay, configAttribs, config, 1, &numConfigs) == EGL_FALSE || numConfigs == 0) {
                LOGE("eglChooseConfig failed or found no configs: %x", eglGetError());
                return false;
            }
        }


        return true;
    }




}
