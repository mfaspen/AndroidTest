//
// Created by DELL on 2025/10/16.
//

#ifndef MY_APPLICATION_INTERFACE_H
#define MY_APPLICATION_INTERFACE_H
#include <stdint.h>
#include <atomic>
#include <iostream>
#define MY_API __attribute__((visibility("default")))


extern "C" {
    MY_API void setPointer(int* unityParam);

    MY_API void*  GetRenderEventFunc();

};

void getContext();

#endif //MY_APPLICATION_INTERFACE_H
