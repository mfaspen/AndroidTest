//
// Created by DELL on 2025/10/20.
//

#ifndef MY_APPLICATION_PRIMARYRENDERER_H
#define MY_APPLICATION_PRIMARYRENDERER_H
#include "GLRenderer.h"

namespace egl{

    class PrimaryRenderer : public GLRenderer {

    protected:
        GLuint CompileShader(GLenum type, const std::string& source);

        GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);

        bool SetupEGL(EGLConfig* config) override;

        bool SetupGL();

        void Rendering() override;

        void update();

        void draw();


    public:

        PrimaryRenderer();
        ~PrimaryRenderer();

    };

}


#endif //MY_APPLICATION_PRIMARYRENDERER_H
