//
// Created by DELL on 2025/10/20.
//

#ifndef MY_APPLICATION_SECONDARYRENDERER_H
#define MY_APPLICATION_SECONDARYRENDERER_H

#include "GLRenderer.h"


namespace egl{

    class SecondaryRenderer : public GLRenderer {

    protected:

        GLuint CompileShader(GLenum type, const std::string& source);

        GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);

        bool SetupEGL(EGLConfig* config) override;

        bool SetupGL();

        void Rendering();

        void Update();

        void Draw();

    public:

        SecondaryRenderer();
        ~SecondaryRenderer();


    };

};


#endif //MY_APPLICATION_SECONDARYRENDERER_H
