package com.example.myappTest

import android.app.Presentation
import android.content.Context
import android.os.Bundle
import android.view.Display
import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.WindowManager
import android.util.Log

class TrianglePresentation(context: Context, display: Display) : Presentation(context, display) {

    companion object {
        init {
            System.loadLibrary("myappTest") // 名称同 CMake 中生成的库
        }
    }

    // native 方法（JNI）
    private external fun nativeInit(surface: Surface)
    private external fun nativeResize(w: Int, h: Int)
    private external fun nativeStart()
    private external fun nativeStop()
    private external fun nativeRelease()

    private var glSurface: SurfaceView? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        // 可选：设置为全屏
        window?.setType(WindowManager.LayoutParams.TYPE_APPLICATION_PANEL)
        setContentView(R.layout.presentation_layout)

        glSurface = findViewById(R.id.surface_view)
        glSurface?.holder?.addCallback(object : SurfaceHolder.Callback {
            override fun surfaceCreated(holder: SurfaceHolder) {
                Log.d("TrianglePresentation", "surfaceCreated")
                nativeInit(holder.surface)
                nativeStart() // 启动渲染线程
            }

            override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
                Log.d("TrianglePresentation", "surfaceChanged: $width x $height")
                nativeResize(width, height)
            }

            override fun surfaceDestroyed(holder: SurfaceHolder) {
                Log.d("TrianglePresentation", "surfaceDestroyed")
                nativeStop()    // 停止渲染线程
                nativeRelease() // 销毁 EGL / 释放 ANativeWindow
            }
        })
    }

    override fun onDisplayRemoved() {
        super.onDisplayRemoved()
        Log.d("TrianglePresentation", "display removed")
    }

    override fun onStop() {
        super.onStop()
        // ensure cleanup
        nativeStop()
        nativeRelease()
    }
}