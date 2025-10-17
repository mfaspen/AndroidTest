#include "native_render.h"
#include <jni.h>
#include <android/native_window_jni.h> // <-- 必须添加这个头文件！
#include <android/native_window.h>
#include <android/log.h>
#include <cmath>
#include <unistd.h>

#define TAG "NativeRenderer"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

#define GL_CALL(x) (x)//do{x;CheckGLError(__FILE__,__LINE__);}while(0)

// 在 native_renderer.cpp 中
#define LOG_THREAD_ID(msg) LOGI("%s: Thread ID: %lu", msg, (long unsigned int)pthread_self())
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

NativeRenderer::NativeRenderer() = default;
NativeRenderer::~NativeRenderer() {
    destroy();
}

// ---------------------- GLSL 着色器代码 ----------------------

const char* VERTEX_SHADER_SOURCE = R"V0G0N(#version 300 es
layout (location = 0) in vec3 aPos;
void main() {
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
)V0G0N";

const char* FRAGMENT_SHADER_SOURCE = R"V0G0N(#version 300 es
precision mediump float;
out vec4 FragColor;
void main() {
    // 画一个红色三角形
    FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
)V0G0N";

// ---------------------- 辅助函数 (编译/链接) ----------------------
EGLDisplay NativeRenderer::m_display =  EGL_NO_DISPLAY;


GLuint NativeRenderer::compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        LOGE("Shader compilation failed (%s): %s", (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"), infoLog);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint NativeRenderer::linkProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        LOGE("Program linking failed: %s", infoLog);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}


// ---------------------- EGL 初始化 ----------------------

bool NativeRenderer::setupEGL(EGLConfig* config) {
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
            EGL_OPENGL_ES3_BIT, // 关键：请求 ES 3.0
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

// ---------------------- GL 资源设置 ----------------------

bool NativeRenderer::setupGL() {
    // 编译并链接着色器
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, VERTEX_SHADER_SOURCE);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER_SOURCE);
    if (vertexShader == 0 || fragmentShader == 0) return false;

    m_program = linkProgram(vertexShader, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (m_program == 0) return false;

    // 三角形顶点数据 (-1.0 到 1.0 的归一化坐标)
    float vertices[] = {
            -0.5f, -0.5f, 0.0f, // 左下角
            0.5f, -0.5f, 0.0f, // 右下角
            0.0f,  0.5f, 0.0f  // 顶部
    };

    GLuint VBO;
    GL_CALL(glGenVertexArrays(1, &m_vao)); // **需要 ES 3.0**
    GL_CALL(glGenBuffers(1, &VBO));

    // 绑定 VAO
    GL_CALL(glBindVertexArray(m_vao));

    // 绑定 VBO，并上传数据
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, VBO));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

    // 配置顶点属性指针
    GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
    GL_CALL(glEnableVertexAttribArray(0));

    // 解绑 VAO (保持 VBO 绑定)
    GL_CALL(glBindVertexArray(0));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    // 注意：VBO 仍然是 VAO 的一部分，这里为了代码简洁不释放 VBO，
    // 在 destroy() 中统一清理。

    LOGI("GL setup complete: VAO created.");
    return true;
}

// ---------------------- 公共接口实现 ----------------------

bool NativeRenderer::primaryInit(ANativeWindow* window) {
    m_window = window;
    if (!m_window) {
        LOGE("ANativeWindow is NULL.");
        return false;
    }

    EGLConfig config;
    if (!setupEGL(&config)) {
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
    setupGL();
    while(true){
        secRender();
        usleep(16000);
    }
    destroy();
    return true;
}
bool NativeRenderer::secondInit(ANativeWindow* window) {
    m_window = window;
    if (!m_window) {
        LOGE("ANativeWindow is NULL.");
        return false;
    }

    EGLConfig config;
    if (!setupEGL(&config)) {
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

    // 5. 创建 EGL Context (请求 ES 3.0)
    EGLint ctxAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    m_context = eglCreateContext(m_display, config, EGL_NO_CONTEXT, ctxAttribs);
    if (m_context == EGL_NO_CONTEXT) {
        LOGE("eglCreateContext failed: %x", eglGetError());
        return false;
    }

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
    setupGL();
    while(true){
        render();
        usleep(16000);
    }
    destroy();
    return true;
}

void NativeRenderer::render() {
    if (m_display == EGL_NO_DISPLAY || m_context == EGL_NO_CONTEXT) {
        LOGE("call to OpenGL ES API with no current context or display!");
        return;
    }


    // 设置视口
    GL_CALL(glViewport(0, 0, m_width, m_height));

    // 清除屏幕
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // 灰蓝色背景
    glClear(GL_COLOR_BUFFER_BIT);

    // 使用着色器程序
    glUseProgram(m_program);

    // 绑定 VAO 并绘制
    GL_CALL(glBindVertexArray(m_vao));
    GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 3));
    GL_CALL(glBindVertexArray(0));
    long long tempTime = get_nano_time();
    // 交换缓冲区，显示到屏幕
    //eglSwapBuffers(m_display, m_surface[0].surface);
    GL_CALL(glFlush());
    LOGE("FLUSH time-consuming:%lld",get_nano_time() - tempTime );

}
void NativeRenderer::secRender() {
    if (m_display == EGL_NO_DISPLAY || m_context == EGL_NO_CONTEXT) {
        LOGE("call to OpenGL ES API with no current context or display!");
        return;
    }


    // 设置视口
    glViewport(0, 0, m_width, m_height);

    // 清除屏幕
    glClearColor(0.2f, 1.3f, 0.3f, 1.0f); // 灰蓝色背景
    glClear(GL_COLOR_BUFFER_BIT);

    // 使用着色器程序
    glUseProgram(m_program);

    // 绑定 VAO 并绘制
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    // 交换缓冲区，显示到屏幕
    //eglSwapBuffers(m_display, m_surface[0].surface);
    //long long tempTime = get_nano_time();
    // 交换缓冲区，显示到屏幕
    //eglSwapBuffers(m_display, m_surface[0].surface);
    GL_CALL(glFlush());
    //LOGE("FLUSH time-consuming:%lld",get_nano_time() - tempTime );
}


void NativeRenderer::destroy() {
    if (m_display != EGL_NO_DISPLAY) {
        eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (m_context != EGL_NO_CONTEXT) {
            eglDestroyContext(m_display, m_context);
        }
        if (m_surface != EGL_NO_SURFACE) {
            eglDestroySurface(m_display, m_surface);
        }
        eglTerminate(m_display);
    }
    if (m_window) {
        ANativeWindow_release(m_window);
        m_window = nullptr;
    }
    m_display = EGL_NO_DISPLAY;
    m_context = EGL_NO_CONTEXT;
    m_surface = EGL_NO_SURFACE;

    // 释放 GL 资源
    if (m_program) glDeleteProgram(m_program);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    LOGI("Renderer destroyed.");
}

// ---------------------- JNI 接口 ----------------------

// 全局渲染器指针
static NativeRenderer* primary_renderer = nullptr;
static NativeRenderer* second_renderer = nullptr;

extern "C" JNIEXPORT jlong JNICALL
Java_com_example_myappTest_NativeRenderer_primaryInitialization(JNIEnv* env, jclass /* clazz */, jobject surface) {

    LOGI("%s: Thread ID: %lu", "init",(long unsigned int)pthread_self()); //
    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    primary_renderer = new NativeRenderer();

    if (!primary_renderer->primaryInit(window)) {
        LOGE("NativeRenderer init failed!");
        delete primary_renderer;
        primary_renderer = nullptr;
    }
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_example_myappTest_NativeRenderer_secondInitialization(JNIEnv* env, jclass /* clazz */, jobject surface) {
    //pthread_self() 获取线程ID
    LOGI("%s: Thread ID: %lu", "init",(long unsigned int)pthread_self()); //
    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    second_renderer = new NativeRenderer();

    if (!second_renderer->secondInit(window)) {
        LOGE("NativeRenderer init failed!");
        delete second_renderer;
        second_renderer = nullptr;
    }
}

