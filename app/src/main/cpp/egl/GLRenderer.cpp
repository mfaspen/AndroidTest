//
// Created by DELL on 2025/10/17.
//

#include <unistd.h>
#include "GLRenderer.h"
#include "PrimaryRenderer.h"
#include "SecondaryRenderer.h"
#include <vector>
#include "slam.h"

namespace egl{

    const char* VERTEX_SHADER_SOURCE = R"(#version 300 es
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 coordinate;
out vec2 V_Texcoord;
void main() {
    V_Texcoord = coordinate;
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
}
)";


    PrimaryRenderer primaryRenderer;
    SecondaryRenderer secondaryRenderer;
    EGLContext unity_context = EGL_NO_CONTEXT;
    EGLContext second_context = EGL_NO_CONTEXT;
    EGLContext primaryContext = EGL_NO_CONTEXT;

    EGLDisplay unityDisplay = EGL_NO_DISPLAY;
    EGLDisplay m_display = EGL_NO_DISPLAY;
    int* unityPara = nullptr;
    EGLConfig eglConfig = nullptr;
    int unityImg[4] = {};
    EGLSurface unitySurface;

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
    bool InitPrimaryRenderer(ANativeWindow* window){
        return primaryRenderer.Initialization(window);
    }

    bool InitSecondaryRenderer(ANativeWindow* window) {
        return secondaryRenderer.Initialization(window);

    }

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
            eglGetConfigAttrib(unityDisplay, configs[i], EGL_CONFIG_ID, &id);LOGE("looking id:%d   numConfigs:%d",id,configID);
            if (id == configID) {
                eglConfig = configs[i];
                break;
            }
        }
    }


    void SetUnityPointer(int* uPara){
        unityPara = uPara;
        LOGE("setUnityPointer success! %d ,%d, %d",unityPara[0],unityPara[1],unityPara[2]);

    }

    void RenderWithUnityTexture(){



        if(unity_context == EGL_NO_CONTEXT){

            EGLContext ctx = eglGetCurrentContext();
            if(ctx != EGL_NO_CONTEXT){
                unity_context =ctx;
                unityDisplay = eglGetCurrentDisplay();
            }
//            if(eglConfig == nullptr){
//                getConfig();
//            }

        }else{

//            if(eglConfig == nullptr){
//                getConfig();
//            }
            egl::unityImg[0] = egl::unityPara[0];
            egl::unityImg[1] = egl::unityPara[1];



        }
        //LOGE("eglContext:%llx,%llx",eglGetCurrentContext(),unity_context);

//        EGLDisplay dpy = eglGetCurrentDisplay();
//        EGLSurface surface = eglGetCurrentSurface(EGL_DRAW);
//
//        if (ctx && dpy && surface)
//            __android_log_print(ANDROID_LOG_INFO, "UnityEGL",
//                                "EGLContext=%p, Display=%p, Surface=%p", ctx, dpy, surface);
//        else
//            __android_log_print(ANDROID_LOG_ERROR, "UnityEGL", "EGL not bound!");
        //initKHR();

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

    bool GLRenderer::Initialization(ANativeWindow *window) {
        m_window = window;
        if (!m_window) {
            LOGE("ANativeWindow is NULL.");
            return false;
        }

        EGLConfig config;
        if (!SetupEGL(&config)) {
            LOGE("EGL setup failed.");
            return false;
        }
        EGLint surfaceAttribs[] = {
                EGL_RENDER_BUFFER,
                EGL_SINGLE_BUFFER,
                EGL_NONE // 必须以 EGL_NONE 结束
        };

        if(m_display == EGL_NO_DISPLAY){
            // 1. 获取 Display
            m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
            if (m_display == EGL_NO_DISPLAY) {
                LOGE("eglGetDisplay failed: %x", eglGetError());
                return false;
            }
        }
        while (unity_context == EGL_NO_CONTEXT
        //||eglConfig == nullptr
        ){
            usleep(16000);
        }
        // 4. 创建 EGL Surface
        m_surface = eglCreateWindowSurface(m_display, config, m_window, surfaceAttribs);
        if (m_surface == EGL_NO_SURFACE) {
            LOGE("eglCreateWindowSurface failed: %x", eglGetError());
            return false;
        }

        //if(m_context == EGL_NO_CONTEXT){
        // 5. 创建 EGL Context (请求 ES 3.0)
        EGLint ctxAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
        primaryContext = eglCreateContext(m_display, config, unity_context, ctxAttribs);
        if (primaryContext == EGL_NO_CONTEXT) {
            LOGE("eglCreateContext failed: %x", eglGetError());
            return false;
        }
        //}

        // 6. 绑定上下文
        if (eglMakeCurrent(m_display, m_surface, m_surface, primaryContext) == EGL_FALSE) {
            LOGE("eglMakeCurrent failed: %x", eglGetError());
            return false;
        }


        CreateTexture11();

        // 获取视口大小
        EGLint width, height;
        eglQuerySurface(m_display, m_surface, EGL_WIDTH, &width);
        eglQuerySurface(m_display, m_surface, EGL_HEIGHT, &height);
        m_width = width;
        m_height = height;
        LOGI("EGL context made current successfully!");

        // 7. 设置 GL 资源
        Rendering();

        return true;
    }

    bool GLRenderer::SetupEGL(EGLConfig* config) {
        if(m_display == EGL_NO_DISPLAY){
            // 1. 获取 Display
            m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
            if (m_display == EGL_NO_DISPLAY) {
                LOGE("eglGetDisplay failed: %x", eglGetError());
                return false;
            }
        }

        // 2. 初始化 EGL
        EGLint major, minor;
        if (eglInitialize(m_display, &major, &minor) == EGL_FALSE) {
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


        EGLint numConfigs;
        if (eglChooseConfig(m_display, attribs, config, 1, &numConfigs) == EGL_FALSE || numConfigs == 0) {
            LOGE("eglChooseConfig failed or found no configs: %x", eglGetError());
            return false;
        }
        return true;
    }





}
