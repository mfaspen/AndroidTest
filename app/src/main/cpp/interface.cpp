//
// Created by DELL on 2025/10/16.
//
#include "interface.h"
#include "GLRenderer.h"

void setPointer(int* unityParam){
    //egl::setUnityPointer(unityParam);
}

void OnRenderEvent(int eventID)
{

    if(egl::unity_context == EGL_NO_CONTEXT){
        egl::unity_context = eglGetCurrentContext();
        egl::unityDisplay = eglGetCurrentDisplay();
        egl::getConfig();
        //egl::CreateTexture11();
    }else{
        //egl::texture11 = eventID;

    }

}

// 这个函数返回一个函数指针，让Unity知道OnRenderEvent在哪
void* GetRenderEventFunc()
{
    return (void*)OnRenderEvent;
}