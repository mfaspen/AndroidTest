package com.example.Application

import android.view.Surface
import kotlin.concurrent.thread

class NativeRenderer{


    private var primaryRenderThread: Thread? = null

    private var secondRenderThread: Thread? = null

    companion object {
        init {
            System.loadLibrary("myappTest") // 名称同 CMake 中生成的库
        }


        @JvmStatic
        private external fun primaryInitialization(surface: Any): Long

        @JvmStatic
        private external fun secondInitialization(surface: Any): Long


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
    }

}