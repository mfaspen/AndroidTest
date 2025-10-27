package com.example.Application

import android.opengl.EGL14
import android.opengl.EGLContext
import android.util.Log
import android.view.Surface
import kotlin.concurrent.thread

class NativeRenderer{


    private var primaryRenderThread: Thread? = null

    private var secondRenderThread: Thread? = null

    companion object {
        init {
            System.loadLibrary("SecondRendering") // 名称同 CMake 中生成的库
        }


        @JvmStatic
        private external fun primaryInitialization(surface: Any): Long

        @JvmStatic
        private external fun secondInitialization(surface: Any): Long

        @JvmStatic
        private external fun nativePause()

        @JvmStatic
        private external fun nativeResume()

        @JvmStatic
        fun getContext() {

            val ctx = EGL14.eglGetCurrentContext()
            if (ctx == EGL14.EGL_NO_CONTEXT) {
                Log.e("TAG", "looking 当前线程没有EGLContext绑定")
            } else {
                Log.i("TAG", "looking 当前EGLContext有效: $ctx")
            }
            Log.i("TAG", "looking EGLContext: $ctx")
            // 在这里处理来自 C++ 的回调逻辑


        }

    }


    /**
     * 初始化主渲染器。
     */
    fun primaryRender(surface: Surface) {
        primaryRenderThread = thread {
             primaryInitialization(surface)
        }
        primaryRenderThread = null
    }


    fun secondRender(surface: Surface) {


        secondRenderThread = thread {
            secondInitialization(surface)
        }
        secondRenderThread = null


//        primaryRenderThread = thread {
//            primaryInitialization(surface)
//        }
//        primaryRenderThread = null
    }

    fun pauseRendering(){
        nativePause();
    }

    fun resumeRendering(){
        nativeResume();
    }
}