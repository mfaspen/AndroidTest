#define EXPORT_RENDER

#ifdef EXPORT_RENDER
#define RENDER_API __declspec(dllexport)
#else
#define RENDER_API __declspec(dllimport)
#endif

#include "NaitveRender.h"   // jni接口
#include "GLRenderer.h"
#include "interface.h"  //c++接口
#include <android/native_window_jni.h> // <-- 必须添加这个头文件！
#include <android/native_window.h>
#include <cmath>




extern "C" JNIEXPORT jlong JNICALL
Java_com_example_Application_NativeRenderer_primaryInitialization(JNIEnv* env, jclass /* clazz */, jobject surface) {

    LOGI("%s: Thread ID: %lu", "init",(long unsigned int)pthread_self()); //
    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);

    if (!egl::InitPrimaryRenderer(window)) {
        LOGE("NativeRenderer init failed!");
    }
    ANativeWindow_release(window);


}

extern "C" JNIEXPORT jlong JNICALL
Java_com_example_Application_NativeRenderer_secondInitialization(JNIEnv* env, jclass /* clazz */, jobject surface) {
    //pthread_self() 获取线程ID
    LOGI("%s: Thread ID: %lu", "init",(long unsigned int)pthread_self()); //
    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);

    if (!egl::InitSecondaryRenderer(window)) {
        LOGE("NativeRenderer init failed!");
    }
    ANativeWindow_release(window);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_Application_PluginBridge_nativeSetSecondarySurface(JNIEnv *env, jobject thiz,
                                                                  jobject surface) {
    // TODO: implement nativeSetSecondarySurface()
}