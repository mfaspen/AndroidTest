//
// Created by DELL on 2025/10/16.
//

#ifndef MY_APPLICATION_MATH_ADD_H
#define MY_APPLICATION_MATH_ADD_H

#include <stdint.h>


class math_add{
private:

    int value = 0;


public:
    math_add(int a, int b);
    ~math_add();
    int get_value();
};

int test_add();
#endif //MY_APPLICATION_MATH_ADD_H
