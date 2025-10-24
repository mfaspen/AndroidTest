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
    // 画一个红色三角形
    FragColor = texture(U_Texture,V_Texcoord);
}
)";

    EGLContext m_context =  EGL_NO_CONTEXT;
    JNIEnv* kt;
    int unityImg[3] = {0,0,0};
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
    EGLContext unityDisplay = EGL_NO_DISPLAY;
    EGLConfig unityConfig = nullptr;

    bool InitPrimaryRenderer(ANativeWindow* window){
        while(unity_context == EGL_NO_CONTEXT){
            usleep(50000);
            LOGE("not obtained eglContext");
        }
        return primaryRenderer.Initialization(window);
    }

    bool InitSecondaryRenderer(ANativeWindow* window) {
        while(unity_context == EGL_NO_CONTEXT){
            usleep(50000);
            LOGE("not obtained eglContext");
        }
        return secondaryRenderer.Initialization(window);

    }
    void getConfig(){

        EGLint configID = 0;
        EGLSurface s = eglGetCurrentSurface(EGL_DRAW);
        eglQuerySurface(unityDisplay, s, EGL_CONFIG_ID, &configID);
        //eglQueryContext(unityDisplay, unity_context, EGL_CONFIG_ID, &configID);

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

    EGLImageKHR eglImage = EGL_NO_IMAGE_KHR;
    GLuint localTex = 0;

// ----- 扩展类型 -----
    typedef void* GLeglImageOES;
    typedef void (GL_APIENTRYP PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)(GLenum target, GLeglImageOES image);

// ----- 函数指针 -----
    static PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = nullptr;
    typedef EGLBoolean (EGLAPIENTRYP PFNEGLDESTROYIMAGEKHRPROC)(EGLDisplay dpy, EGLImageKHR image);
    static PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHRFunc = nullptr;

    void updateKHR(){

        PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR =
                (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
        PFNGLEGLIMAGETARGETTEXTURE2DOESPROC  glEGLImageTargetTexture2DOES  =
                (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");

        if (!glEGLImageTargetTexture2DOES) {
            LOGE("glEGLImageTargetTexture2DOES not found!");
            return;
        }

        if (eglImage != EGL_NO_IMAGE_KHR) {
            eglDestroyImageKHRFunc(unityDisplay, eglImage);
            eglImage = EGL_NO_IMAGE_KHR;
        }

        eglImage = eglCreateImageKHR(
                unityDisplay,
                unity_context,               // 可以用 EGL_NO_CONTEXT
                EGL_GL_TEXTURE_2D_KHR,
                (EGLClientBuffer)(uintptr_t)unityPara[0],
                nullptr);

        if (eglImage == EGL_NO_IMAGE_KHR) {
            LOGE("eglCreateImageKHR failed!");
            return;
        }

        if (localTex == 0)
            glGenTextures(1, &localTex);

        glBindTexture(GL_TEXTURE_2D, localTex);
        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, eglImage);
        LOGI("Created local texture %u from Unity texture %u", localTex, unityPara[0]);
    }
    void RenderWithUnityTexture(){
        updateKHR();

        if(unity_context == EGL_NO_CONTEXT){

            EGLContext ctx = eglGetCurrentContext();
            EGLDisplay dpy = eglGetCurrentDisplay();

//            EGLint versionMajor, versionMinor;
//            eglInitialize(dpy, &versionMajor, &versionMinor);
//
//            __android_log_print(ANDROID_LOG_INFO, "TAG",
//                                "display=%p ctx=%p surf=%p EGL version %d.%d",
//                                dpy, ctx, s, versionMajor, versionMinor);

            if(ctx != EGL_NO_CONTEXT){
                unity_context =ctx;
                unityDisplay = dpy;
            }
            if(unityConfig == nullptr){
                getConfig();
            }

        }else{

            if(unityConfig == nullptr){
                getConfig();
            }
            egl::unityImg[0] = egl::unityPara[0];



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




    GLuint texture11;
    int* unityPara;
    bool paused = false;
    void setUnityPointer(int* uPara){
        unityPara = uPara;
        LOGE("setUnityPointer success! %d ,%d, %d",unityPara[0],unityPara[1],unityPara[2]);

    }

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

    /*
     * 初始化surface
     */

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

        // 4. 创建 EGL Surface
        m_surface = eglCreateWindowSurface(unityDisplay, unityConfig, m_window, surfaceAttribs);


        if (m_surface == EGL_NO_SURFACE) {
            LOGE("eglCreateWindowSurface failed: %x", eglGetError());
            return false;
        }

        //if(m_context == EGL_NO_CONTEXT){

        EGLint ctxAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
        m_context = eglCreateContext(unityDisplay, unityConfig, unity_context, ctxAttribs);
        if (m_context == EGL_NO_CONTEXT) {
            LOGE("eglCreateContext failed: %x", eglGetError());
            return false;
        }
        //}


        // 6. 绑定上下文
        if (eglMakeCurrent(unityDisplay, m_surface, m_surface, m_context) == EGL_FALSE) {
            LOGE("eglMakeCurrent failed: %x", eglGetError());
            return false;
        }


        // 获取视口大小
        EGLint width, height;
        eglQuerySurface(unityDisplay, m_surface, EGL_WIDTH, &width);
        eglQuerySurface(unityDisplay, m_surface, EGL_HEIGHT, &height);
        m_width = width;
        m_height = height;
        LOGI("EGL context made current successfully!");

        CreateTexture11();
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
                EGL_RED_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_ALPHA_SIZE, 8,
                EGL_DEPTH_SIZE, 16,
                EGL_NONE
        };


        EGLint numConfigs;
        if (eglChooseConfig(unityDisplay, attribs, config, 1, &numConfigs) == EGL_FALSE || numConfigs == 0) {
            LOGE("eglChooseConfig failed or found no configs: %x", eglGetError());
            return false;
        }
        return true;
    }




}
