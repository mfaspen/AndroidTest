//
// Created by DELL on 2025/10/16.
//
#include "interface.h"
#include "GLRenderer.h"
#include "NaitveRender.h"

#include <EGL/egl.h>
#include <GLES3/gl3.h>

void setPointer(int* unityParam){
    egl::setUnityPointer(unityParam);
}

void OnRenderEvent(int eventID)
{




    switch (eventID)
    {
        case 1:
            egl :: RenderWithUnityTexture();
            break;
        case 2:
            // 例如 stop / cleanup
            break;
        default:
            break;
    }
}
// 这个函数返回一个函数指针，让Unity知道OnRenderEvent在哪
void*  GetRenderEventFunc()
{
    return (void*)OnRenderEvent;
}
