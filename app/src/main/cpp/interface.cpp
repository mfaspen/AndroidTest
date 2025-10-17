//
// Created by DELL on 2025/10/16.
//
#include "interface.h"

interface::interface(int a , int b ){
    value = a+b;
}

interface::~interface(){

}

int interface::get_value() {
    return value;
}


int test_add(){
    return 10;
}