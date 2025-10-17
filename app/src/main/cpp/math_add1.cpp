//
// Created by DELL on 2025/10/16.
//
#include "math_add.h"

math_add::math_add(int a , int b ){
    value = a+b;
}

math_add::~math_add(){

}

int math_add::get_value() {
    return value;
}


int test_add(){
    return 10;
}