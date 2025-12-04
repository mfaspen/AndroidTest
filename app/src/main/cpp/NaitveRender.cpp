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
#include "slam.h"



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
extern "C" JNIEXPORT jlong JNICALL
Java_com_example_Application_NativeRenderer_nAddUsbDevice(JNIEnv* env, jclass /* clazz */, jstring
deviceName_,jint fileDescriptor) {
    //pthread_self() 获取线程ID

//    static bool firstCall = true;
//    if (firstCall) {
//        firstCall = false;
//        std::cout.rdbuf(new androidout);
//        std::cerr.rdbuf(new androiderr);
//        env->GetJavaVM(&jvm);
//        std::cout << "Initialized" << std::endl;
//    }
        slam::init(fileDescriptor);
//    int fd = fileDescriptor;
//    LOG_DEBUG("nAddUsbDevice fd: %d", fd);
//
//    device = xv::getDevice(fd);
//    xv::setLogLevel(xv::LogLevel(0));
//    if (!device) {
//        LOG_DEBUG("nAddUsbDevice getDevice FAIL");
//        return;
//    }
//
//    m_ready = true;
//    usleep(2000 * 1000);
//    LOG_DEBUG("nAddUsbDevice inited opencv version:%s", cv::getVersionString().c_str());

}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_Application_PluginBridge_nativeSetSecondarySurface(JNIEnv *env, jobject thiz,
                                                                  jobject surface) {
    // TODO: implement nativeSetSecondarySurface()
}