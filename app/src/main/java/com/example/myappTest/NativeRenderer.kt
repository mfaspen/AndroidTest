package com.example.myappTest

import android.view.Surface

class NativeRenderer{

    var primaryHandle:Long = 0L
        private set


    companion object {
        init {
            System.loadLibrary("myappTest") // 名称同 CMake 中生成的库
        }


        @JvmStatic
        private external fun nativeInit(surface: Any): Long

        @JvmStatic
        private external fun nativeRender( primaryHandle:Long)

        @JvmStatic
        private external fun nativeInitSecondary(handle: Long, surface: Surface): Long // 假设返回副屏句柄

        @JvmStatic
        private external fun nativeDestroy()

    }

    /**
     * 初始化主渲染器。
     */
    fun init(surface: Surface) {
        primaryHandle = nativeInit(surface)
        if (primaryHandle == 0L) {
            throw RuntimeException("NativeRenderer initialization failed!")
        }
    }

    /**
     * 渲染主屏幕。
     */
    fun renderPrimary(){
        if(primaryHandle!=0L){
            nativeRender(primaryHandle)
        }
    }



    // 传统的 NativeRenderer 类现在成了 JNI 接口的集合
    // nativeInit 返回 C++ 实例的句柄
//    @JvmStatic
//    private external fun nativeInit(surface: Surface): Long
//
//
//    // 所有 JNI 方法现在都需要 handle 作为第一个参数
//    @JvmStatic
//    private external fun nativeRender(handle: Long)


    // ... 其他 native 方法

}