#include <dlfcn.h>
#include <map>
#include "getSlam.h"

#include <android/log.h>
#define TAG "NativeRenderer"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

//
// Created by DELL on 2025/10/17.
//

slamParameter::slamParameter() {

}

slamParameter::~slamParameter() {

}

void* handle = dlopen("libOtherLib.so", RTLD_NOW);



void getFunction(){

    if (!handle) {
        LOGI("dlopen failed: %s", dlerror());
    return;
    }

    // 找到函数指针
    typedef int (*GetValueFunc)(int);
    GetValueFunc getValue = (GetValueFunc)dlsym(handle, "getValueFromOther");

    if (getValue) {
    int result = getValue(42);
        LOGI("Result = %d", result);
    } else {
        LOGI("dlsym failed: %s", dlerror());
    }

    dlclose(handle);
}