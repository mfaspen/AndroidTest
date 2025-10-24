//
// Created by DELL on 2025/10/20.
//

#include "SecondaryRenderer.h"

namespace egl{

    SecondaryRenderer::SecondaryRenderer(){

    }


    int renderVsync = 0;
    void SecondaryRenderer::Rendering(){
        SetupGL();
        while(true){
            renderVsync ++;
            if (paused) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }
            Update();
            Draw();
            usleep(16000);
        }

    }

    void SecondaryRenderer::Draw(){


        // setup sampling params if needed



        if (unityDisplay == EGL_NO_DISPLAY || m_context == EGL_NO_CONTEXT) {
            LOGE("call to OpenGL ES API with no current context or display!");
            return;
        }

        glViewport(0,0,m_width/2,m_height);

        // 清除屏幕
        glClearColor(0.2f, 1.3f, 0.3f, 1.0f); // 灰蓝色背景
        glClear(GL_COLOR_BUFFER_BIT);

        // 使用着色器程序
        glUseProgram(m_program);

        glActiveTexture(GL_TEXTURE0);

        if(renderVsync % 200 > 100){
            glBindTexture(GL_TEXTURE_2D,localTex);
        }else{

            glBindTexture(GL_TEXTURE_2D,texture11);
        }
        // 绑定 VAO 并绘制
        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // 交换缓冲区，显示到屏幕
        //eglSwapBuffers(unityDisplay, m_surface[0].surface);
        //long long tempTime = get_nano_time();
        // 交换缓冲区，显示到屏幕
        //eglSwapBuffers(unityDisplay, m_surface[0].surface);
        //LOGE("FLUSH time-consuming:%lld",get_nano_time() - tempTime );
        glViewport(m_width/2,0,m_width/2,m_height);
        if (unityDisplay == EGL_NO_DISPLAY || m_context == EGL_NO_CONTEXT) {
            LOGE("call to OpenGL ES API with no current context or display!");
            return;
        }
        // 使用着色器程序
        glUseProgram(m_program);

        // 绑定 VAO 并绘制
        glActiveTexture(GL_TEXTURE0);
        if(renderVsync % 200 > 100){
            glBindTexture(GL_TEXTURE_2D,texture11);
        }else{

            glBindTexture(GL_TEXTURE_2D,localTex);
        }
        int w,h;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

        LOGE("looking glGetTexLevelParameteriv %d   %d",w,h);
        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // 交换缓冲区，显示到屏幕
        //eglSwapBuffers(unityDisplay, m_surface[0].surface);
        //long long tempTime = get_nano_time();
        // 交换缓冲区，显示到屏幕
        //eglSwapBuffers(unityDisplay, m_surface[0].surface);
        GL_CALL(glFlush());
        //LOGE("FLUSH time-consuming:%lld",get_nano_time() - tempTime );

        LOGE("unityPara 3 : %d",unityPara[0]);

    }

    bool SecondaryRenderer::SetupEGL(EGLConfig* config) {
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


    bool SecondaryRenderer::SetupGL() {
        // 编译并链接着色器
        GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, VERTEX_SHADER_SOURCE);
        GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER_SOURCE);
        if (vertexShader == 0 || fragmentShader == 0) return false;

        m_program = LinkProgram(vertexShader, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        if (m_program == 0) return false;

        // 三角形顶点数据 (-1.0 到 1.0 的归一化坐标)
        float vertices[] = {
                -0.9f, -0.9f, 0.0f,0.0f,0.0f, // 左下角
                0.9f, -0.9f, 0.0f, 1.0f,0.0f,// 右下角
                -0.9f,  0.9f, 0.0f, 0.0f,1.0f, // 左上角
                -0.9f,  0.9f, 0.0f,  0.0f,1.0f,// 左上角
                0.9f, -0.9f, 0.0f, 1.0f,0.0f,// 右下角
                0.9f,  0.9f, 0.0f,  1.0f,1.0f,// 右上角
        };

        GLuint VBO;
        GL_CALL(glGenVertexArrays(1, &m_vao)); // **需要 ES 3.0**
        GL_CALL(glGenBuffers(1, &VBO));

        // 绑定 VAO
        GL_CALL(glBindVertexArray(m_vao));

        // 绑定 VBO，并上传数据
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, VBO));
        GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

        // 配置顶点属性指针  1:位置  2：多少数据 3：数据类型 4:是否为0-255 (0-1)  5 :总大小 6:数据偏移
        GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0));
        GL_CALL(glEnableVertexAttribArray(0));
        GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof (float)*3)));
        GL_CALL(glEnableVertexAttribArray(1));

        // 解绑 VAO (保持 VBO 绑定)
        GL_CALL(glBindVertexArray(0));
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
        // 注意：VBO 仍然是 VAO 的一部分，这里为了代码简洁不释放 VBO，
        // 在 destroy() 中统一清理。

        LOGI("GL setup complete: VAO created.");
        return true;
    }







    GLuint SecondaryRenderer::CompileShader(GLenum type, const std::string& source) {
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

    GLuint SecondaryRenderer::LinkProgram(GLuint vertexShader, GLuint fragmentShader) {
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



    void SecondaryRenderer::Update(){



    }



    SecondaryRenderer::~SecondaryRenderer() {
        if (unityDisplay != EGL_NO_DISPLAY) {
            eglMakeCurrent(unityDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            if (m_context != EGL_NO_CONTEXT) {
                eglDestroyContext(unityDisplay, m_context);
            }
            if (m_surface != EGL_NO_SURFACE) {
                eglDestroySurface(unityDisplay, m_surface);
            }
            eglTerminate(unityDisplay);
        }
        unityDisplay = EGL_NO_DISPLAY;
        m_context = EGL_NO_CONTEXT;
        m_surface = EGL_NO_SURFACE;

        // 释放 GL 资源
        if (m_program) glDeleteProgram(m_program);
        if (m_vao) glDeleteVertexArrays(1, &m_vao);
        LOGI("Renderer destroyed.");
    }


}