package com.example.Application

import android.view.Surface
import com.example.Application.NativeRenderer.Companion.nAddUsbDevice
import kotlin.concurrent.thread

class NativeRenderer{


    private var primaryRenderThread: Thread? = null

    private var secondRenderThread: Thread? = null

    private var deviceThread: Thread? = null

    companion object {


        init {
            System.loadLibrary("SecondRendering") // 名称同 CMake 中生成的库
        }


        @JvmStatic
        private external fun primaryInitialization(surface: Any): Long

        @JvmStatic
        private external fun secondInitialization(surface: Any): Long

        @JvmStatic
        private external fun nAddUsbDevice(deviceName: String?, fileDescriptor: Int)


    }

    /**
     * 初始化主渲染器。
     */
    fun primaryRender(surface: Surface) {
        primaryRenderThread = thread {
            android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_DISPLAY)
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

    fun addUsbDevice(deviceName: String?, fileDescriptor: Int) {
        deviceThread = thread{
            nAddUsbDevice(deviceName, fileDescriptor)
        }
    }
}