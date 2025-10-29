//
// Created by DELL on 2025/10/16.
//

#ifndef MY_APPLICATION_INTERFACE_H
#define MY_APPLICATION_INTERFACE_H
#define MY_API __attribute__((visibility("default")))
#include <stdint.h>


#include <atomic>
#include <iostream>


extern "C" {

    MY_API void* GetRenderEventFunc();
    MY_API void setPointer(int* id);

};

#endif //MY_APPLICATION_INTERFACE_H
