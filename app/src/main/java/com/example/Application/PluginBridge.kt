package com.example.Application

import android.app.Activity
import android.content.Context
import android.hardware.display.DisplayManager
import android.util.Log
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.widget.FrameLayout

object PluginBridge{
    private var activePresentation: MyPresentation? = null
    private var primaryHandle = NativeRenderer()
    // -----------------------------------------------------------------
    // (A) NDK C++ JNI 声明：将 Surface 传递给 C++
    // -----------------------------------------------------------------
//    init {
//        System.loadLibrary("native-lib")
//    }

//    external  fun nativeSetSecondarySurface(surface: Surface?)

    // -----------------------------------------------------------------
    // (B) C# 调用入口 1: 显示副屏
    // @JvmStatic 确保 C# 可以找到这个静态方法
    // -----------------------------------------------------------------
    @JvmStatic
    fun startPresentation(activity: Activity){
        Log.d("looking","startPresentation")
        val surfaceView = SurfaceView(activity)
        surfaceView.holder.addCallback(object :SurfaceHolder.Callback {
            // --- SurfaceHolder.Callback 实现 ---

            override fun surfaceCreated(holder: SurfaceHolder) {
                primaryHandle.primaryRender(holder.surface)
            }

            override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
                // 可以在这里处理窗口大小变化，但在这个例子中，C++ render() 会查询新的大小
            }

            override fun surfaceDestroyed(holder: SurfaceHolder) {
            }


        })
        // 把 SurfaceView 添加到 Activity 的界面上
        activity.runOnUiThread {
            activity.addContentView(
                surfaceView,
                FrameLayout.LayoutParams(
                    FrameLayout.LayoutParams.MATCH_PARENT,
                    FrameLayout.LayoutParams.MATCH_PARENT
                )
            )
        }


        val displayManager = activity.getSystemService(Context.DISPLAY_SERVICE) as DisplayManager
        if(displayManager.displays.count()>1){

            val displays = displayManager.displays[1]

            activePresentation = MyPresentation(activity,displays)
//
            activePresentation?.show()

        }
    }
}