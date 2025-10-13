#include "native_render.h"
#include <jni.h>
#include <android/native_window_jni.h> // <-- 必须添加这个头文件！
#include <android/native_window.h>
#include <android/log.h>
#include <cmath>

#define TAG "NativeRenderer"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
// 在 native_renderer.cpp 中
#define LOG_THREAD_ID(msg) LOGI("%s: Thread ID: %lu", msg, (long unsigned int)pthread_self())

NativeRenderer::NativeRenderer() = default;
NativeRenderer::~NativeRenderer() {
    destroy();
}

// ---------------------- GLSL 着色器代码 ----------------------

const char* VERTEX_SHADER_SOURCE = R"V0G0N(
#version 300 es
layout (location = 0) in vec3 aPos;
void main() {
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
)V0G0N";

const char* FRAGMENT_SHADER_SOURCE = R"V0G0N(
#version 300 es
precision mediump float;
out vec4 FragColor;
void main() {
    // 画一个红色三角形
    FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
)V0G0N";

// ---------------------- 辅助函数 (编译/链接) ----------------------



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
    // 1. 获取 Display
    m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (m_display == EGL_NO_DISPLAY) {
        LOGE("eglGetDisplay failed: %x", eglGetError());
        return false;
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
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT, // 关键：请求 ES 3.0
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
    glGenVertexArrays(1, &m_vao); // **需要 ES 3.0**
    glGenBuffers(1, &VBO);

    // 绑定 VAO
    glBindVertexArray(m_vao);

    // 绑定 VBO，并上传数据
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 配置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 解绑 VAO (保持 VBO 绑定)
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // 注意：VBO 仍然是 VAO 的一部分，这里为了代码简洁不释放 VBO，
    // 在 destroy() 中统一清理。

    LOGI("GL setup complete: VAO created.");
    return true;
}

// ---------------------- 公共接口实现 ----------------------

bool NativeRenderer::init(ANativeWindow* window) {
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

    // 4. 创建 EGL Surface
    m_surface = eglCreateWindowSurface(m_display, config, m_window, nullptr);
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
    LOGI("EGL context made current successfully!");

    // 7. 设置 GL 资源
    return setupGL();
}

void NativeRenderer::render() {
    if (m_display == EGL_NO_DISPLAY || m_context == EGL_NO_CONTEXT) {
        LOGE("call to OpenGL ES API with no current context or display!");
        return;
    }

    // 获取视口大小
    EGLint width, height;
    eglQuerySurface(m_display, m_surface, EGL_WIDTH, &width);
    eglQuerySurface(m_display, m_surface, EGL_HEIGHT, &height);

    // 设置视口
    glViewport(0, 0, width, height);

    // 清除屏幕
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // 灰蓝色背景
    glClear(GL_COLOR_BUFFER_BIT);

    // 使用着色器程序
    glUseProgram(m_program);

    // 绑定 VAO 并绘制
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    // 交换缓冲区，显示到屏幕
    eglSwapBuffers(m_display, m_surface);
}

void NativeRenderer::nativeRenderSecondary() {
    // 1. 切换到副屏的上下文
    if (eglMakeCurrent(m_display, m_surface[1].surface, m_surface[1].surface, m_context /* 或 sharedContext */) == EGL_FALSE) {
        // 如果这里失败，需要处理错误
        return;
    }

    // 2. 执行渲染：
    // 使用主屏已链接的程序 ID
    glUseProgram(m_program);

    // 设置视口 (使用 secondaryState.width 和 height)
    glViewport(0, 0, m_surface[1].width, m_surface[1].height);

    // 清除副屏颜色 (可以与主屏不同)
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f); // 例如，蓝色
    glClear(GL_COLOR_BUFFER_BIT);

    // 绘制三角形
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    // 3. 交换缓冲区
    eglSwapBuffers(m_display, m_surface[1].surface);

    // 4. 【重要】渲染完成后解绑上下文
    eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
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
static NativeRenderer* g_renderer = nullptr;

extern "C" JNIEXPORT jlong JNICALL
Java_com_example_myappTest_NativeRenderer_nativeInit(JNIEnv* env, jclass /* clazz */, jobject surface) {
    if (g_renderer) return (jlong)g_renderer;
    //pthread_self() 获取线程ID
    LOGI("%s: Thread ID: %lu", "init",(long unsigned int)pthread_self()); //
    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    g_renderer = new NativeRenderer();

    if (!g_renderer->init(window)) {
        LOGE("NativeRenderer init failed!");
        delete g_renderer;
        g_renderer = nullptr;
    }
    return (jlong)g_renderer;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_myappTest_NativeRenderer_nativeRender(JNIEnv* env, jclass /* clazz */,jlong primaryHandle) {
    LOGI("%s: Thread ID: %lu", "render",(long unsigned int)pthread_self()); //
    if (g_renderer) {
        g_renderer->render();
    } else {
        LOGE("nativeRender called with no renderer initialized!");
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_myappTest_NativeRenderer_nativeDestroy(JNIEnv* env, jclass /* clazz */) {
if (g_renderer) {
g_renderer->destroy();
delete g_renderer;
g_renderer = nullptr;
}
}
static JavaVM* g_javaVM;
// 副屏初始化也一样
extern "C" JNIEXPORT jlong JNICALL
Java_com_example_myappTest_NativeRenderer_nativeInitSecondary(JNIEnv* env, jclass clazz, jlong handle, jobject surface) {
    // 1. 转换句柄

    NativeRenderer* renderer = g_renderer;
    if (renderer) {
        // 2. 调用副屏初始化，返回副屏的 EGLSurface 句柄（如果需要）
        return (jlong)renderer;
    }
    return 0;
}