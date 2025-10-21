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
void main() {
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
)";

    const char* FRAGMENT_SHADER_SOURCE = R"(#version 300 es
precision mediump float;
out vec4 FragColor;
void main() {
    // 画一个红色三角形
    FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
)";

    EGLContext m_context =  EGL_NO_CONTEXT;

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

    bool InitPrimaryRenderer(ANativeWindow* window){
        return primaryRenderer.Initialization(window);
    }

    bool InitSecondaryRenderer(ANativeWindow* window) {
        return secondaryRenderer.Initialization(window);

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

        // 4. 创建 EGL Surface
        m_surface = eglCreateWindowSurface(m_display, config, m_window, surfaceAttribs);
        if (m_surface == EGL_NO_SURFACE) {
            LOGE("eglCreateWindowSurface failed: %x", eglGetError());
            return false;
        }

        //if(m_context == EGL_NO_CONTEXT){
        // 5. 创建 EGL Context (请求 ES 3.0)
        EGLint ctxAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
        m_context = eglCreateContext(m_display, config, EGL_NO_CONTEXT, ctxAttribs);
        if (m_context == EGL_NO_CONTEXT) {
            LOGE("eglCreateContext failed: %x", eglGetError());
            return false;
        }
        //}

        // 6. 绑定上下文
        if (eglMakeCurrent(m_display, m_surface, m_surface, m_context) == EGL_FALSE) {
            LOGE("eglMakeCurrent failed: %x", eglGetError());
            return false;
        }


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
//        if(m_display == EGL_NO_DISPLAY){
//            // 1. 获取 Display
//            m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
//            if (m_display == EGL_NO_DISPLAY) {
//                LOGE("eglGetDisplay failed: %x", eglGetError());
//                return false;
//            }
//        }
//
//        // 2. 初始化 EGL
//        EGLint major, minor;
//        if (eglInitialize(m_display, &major, &minor) == EGL_FALSE) {
//            LOGE("eglInitialize failed: %x", eglGetError());
//            return false;
//        }
//        LOGI("EGL initialized. Version %d.%d", (int)major, (int)minor);
//
//        // 3. 配置 EGL 属性 (请求 ES 3.0)
//        EGLint attribs[] = {
//                EGL_RENDERABLE_TYPE,
//                EGL_OPENGL_ES2_BIT, // 关键：请求 ES 3.0
//                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
//                EGL_BLUE_SIZE, 8,
//                EGL_GREEN_SIZE, 8,
//                EGL_RED_SIZE, 8,
//                EGL_DEPTH_SIZE, 16,
//                EGL_NONE
//        };
//
//
//        EGLint numConfigs;
//        if (eglChooseConfig(m_display, attribs, config, 1, &numConfigs) == EGL_FALSE || numConfigs == 0) {
//            LOGE("eglChooseConfig failed or found no configs: %x", eglGetError());
//            return false;
//        }
        return true;
    }




}
