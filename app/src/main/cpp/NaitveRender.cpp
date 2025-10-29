#define EXPORT_RENDER

#ifdef EXPORT_RENDER
#define RENDER_API __declspec(dllexport)
#else
#define RENDER_API __declspec(dllimport)
#endif

#include "NaitveRender.h"   // jni接口
#include "GLRenderer.h"
#include <android/native_window_jni.h> // <-- 必须添加这个头文件！
#include <android/native_window.h>
#include <cmath>



static JavaVM* gJvm = NULL;
static jobject gGlobalSurface = NULL; // global ref to java Surface
static ANativeWindow* gWindow = NULL;


extern "C" JNIEXPORT void JNICALL
 Java_com_example_Application_NativeRenderer_onSurfaceDestroyed() {
    egl::SurfaceDestroyed();
}



extern "C" JNIEXPORT jlong JNICALL
Java_com_example_Application_NativeRenderer_primaryInitialization(JNIEnv* env, jclass /* clazz */, jobject surface) {

    // if there is an existing window, release it first
    if (gWindow) {
        ANativeWindow_release(gWindow);
        gWindow = NULL;
    }
    if (surface == NULL) {
        LOGE("nativeSetSurface called with NULL");
        return 0 ;
    }

//    // Create a global ref so other threads can use it safely
//    gGlobalSurface = env->NewGlobalRef(surface);
//    if (gGlobalSurface == NULL) {
//        LOGE("Failed to create global ref for surface");
//        return 0 ;
//    }

    // Immediately create ANativeWindow on this thread (UI thread)
    gWindow = ANativeWindow_fromSurface(env, surface);
    if (gWindow == NULL) {
        LOGE("ANativeWindow_fromSurface returned NULL");
        // keep global ref so we can try later; but consider deleting the global ref if unrecoverable
    } else {
        LOGI("ANativeWindow created: %p", gWindow);
    }

    if (!egl::InitPrimaryRenderer(gWindow)) {
        LOGE("NativeRenderer init failed!");
    }


    ANativeWindow_release(gWindow);


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