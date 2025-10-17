//
// Created by DELL on 2025/10/17.
//

#ifndef MY_APPLICATION_GETSLAM_H
#define MY_APPLICATION_GETSLAM_H


class slamParameter{
    slamParameter();
    ~slamParameter();
    void getPose(double* pose);

};

typedef int (*GetValueFunc)(int);
GetValueFunc getValue;


#endif //MY_APPLICATION_GETSLAM_H
