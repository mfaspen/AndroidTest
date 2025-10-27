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


static JavaVM *g_JavaVM = nullptr;
jclass g_NativeRendererClass = nullptr;


extern "C" JNIEXPORT jlong JNICALL
Java_com_example_Application_NativeRenderer_primaryInitialization(JNIEnv* env, jclass /* clazz */, jobject surface) {

    LOGI("%s: Thread ID: %lu", "init",(long unsigned int)pthread_self()); //
    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);

    if (!egl::InitPrimaryRenderer(window)) {
        LOGE("NativeRenderer init failed!");
    }
    ANativeWindow_release(window);


}

extern "C" JNIEXPORT void JNICALL
Java_com_example_Application_NativeRenderer_secondInitialization(JNIEnv* env, jclass /* clazz */, jobject surface) {
    //pthread_self() 获取线程ID
    // 从 JNIEnv* 获取 JavaVM*
    if (env->GetJavaVM(&g_JavaVM) != JNI_OK) {
        // 获取失败

        return ;
    }
    jclass localClass = env->FindClass("com/example/Application/NativeRenderer");
    if (localClass == nullptr) {
        LOGI("Error: Class com/example/Application/NativeRenderer not found!");

        return;
    }
    // 2. 将本地引用提升为全局引用并缓存
    g_NativeRendererClass = (jclass)env->NewGlobalRef(localClass);

    // 3. 清除本地引用
    env->DeleteLocalRef(localClass);


    LOGI("%s: looking Thread ID: %lu", "init",(long unsigned int)pthread_self()); //
    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    if (!egl::InitSecondaryRenderer(window)) {
        LOGE("NativeRenderer init failed!");
    }

    ANativeWindow_release(window);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_Application_NativeRenderer_nativePause(JNIEnv* env, jclass /* clazz */) {
    //pthread_self() 获取线程ID
    egl::paused = true;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_Application_NativeRenderer_nativeResume(JNIEnv* env, jclass /* clazz */) {
    //pthread_self() 获取线程ID
    egl::paused = false;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_Application_PluginBridge_nativeSetSecondarySurface(JNIEnv *env, jobject thiz,
                                                                  jobject surface) {
    // TODO: implement nativeSetSecondarySurface()
}


void getContext(){

    LOGI("looking 1");
    while(g_NativeRendererClass == nullptr){
        usleep(50000);
    }
    JNIEnv* env;
    jint result = JNI_OK;
    bool attachedHere = false; // 标记我们是否是附加线程的人
    result = g_JavaVM->GetEnv((void**)&env, JNI_VERSION_1_6);

    if (result == JNI_EDETACHED) {
        // 3. 线程未附加，手动附加
        LOGI("JNI: Thread not attached, attempting to attach...");
        result = g_JavaVM->AttachCurrentThread(&env, NULL);
        attachedHere = true; // 标记我们附加了线程

        if (result != JNI_OK) {
            LOGI("JNI Error: Failed to attach current thread!");
            return;
        }
    } else if (result != JNI_OK) {
        // 4. 其他 JNI 错误
        LOGI("JNI Error: Failed to get JNIEnv, result code: %d", result);
        return;
    }

    jmethodID methodID = env->GetStaticMethodID(
            g_NativeRendererClass,
            "getContext", // 方法名
            "()V"   // 方法签名
    );
    LOGI("looking 4");

    if (methodID == nullptr) {
        LOGI("Error: Static method onNativeMessageReceived not found or signature invalid!");
        return;
    }

    // 3. 准备参数
    const char* message_c = "Hello from C++ Native Code!";
    jstring j_message = env->NewStringUTF(message_c);
    jint j_value = 42;

// 3. 调用 Kotlin/Java 静态方法 (不需要准备任何参数)
    env->CallStaticVoidMethod(
            g_NativeRendererClass,
            methodID
            // 注意：后面不需要跟任何参数了
    );

    // 5. 释放局部引用
    env->DeleteLocalRef(j_message);

    // 注意：对于静态方法，不需要删除 LocalRef 的 Class 对象 (callbackClass)，
    // 因为它是 JNI 查找的全局引用，除非你明确调用了 NewGlobalRef。

    LOGI("Successfully called Kotlin function.");

}