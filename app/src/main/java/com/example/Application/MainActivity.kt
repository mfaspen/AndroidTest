package com.example.Application

import android.content.Context
import android.hardware.display.DisplayManager
import android.os.Bundle
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.View
import android.view.Window
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.WindowCompat
import androidx.core.view.WindowInsetsCompat
import androidx.core.view.WindowInsetsControllerCompat

/**
 * 隐藏状态栏并实现沉浸式全屏
 *
 * @param window 当前 Activity 的 Window 对象
 * @param view 用于获取 WindowInsetsControllerCompat 的 DecorView 或任何 View
 */
fun hideStatusBar(window: Window, view: View) {

    // 1. 设置内容延伸到系统栏区域
    // 这是实现沉浸式全屏的关键一步，让您的内容可以画到状态栏后面
    WindowCompat.setDecorFitsSystemWindows(window, false)

    // 2. 获取 WindowInsetsControllerCompat
    val controller = WindowCompat.getInsetsController(window, view) ?: return

    // 3. 配置隐藏时的行为
    // BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE: 用户从边缘滑动会临时显示系统栏，
    //                                         几秒后自动隐藏。非常适合沉浸式应用。
    controller.systemBarsBehavior = WindowInsetsControllerCompat.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE

    // 4. 隐藏状态栏
    // 使用 WindowInsetsCompat.Type.statusBars() 明确只隐藏状态栏
    controller.hide(WindowInsetsCompat.Type.statusBars())

    // 如果想同时隐藏导航栏，可以使用：
    controller.hide(WindowInsetsCompat.Type.systemBars()) // statusBars() | navigationBars()

}

class MainActivity : AppCompatActivity(){

    // 假设你的布局文件包含一个 id 为 'surfaceView' 的 SurfaceView
    private lateinit var myPresentation : MyPresentation
    private lateinit var surfaceView: SurfaceView
    private var isRendering = false
    private var renderThread: Thread? = null
    private var primaryHandle = NativeRenderer()



    //private var secondaryDisplayPresentation:NaviteRenderer

//    private fun checkForSecondaryDisplay() {
//        val displayManager = getSystemService(Context.DISPLAY_SERVICE) as DisplayManager
//        val displays = displayManager.getDisplays(DisplayManager.DISPLAY_CATEGORY_PRESENTATION)
//
//        if (displays.isNotEmpty() && secondaryDisplayPresentation == null && primaryRenderer != null) {
//            val secondaryDisplay = displays.last()
//
//            // 【关键】将主渲染器实例传入 MyPresentation 的构造函数
//            secondaryDisplayPresentation = MyPresentation(this, secondaryDisplay, primaryRenderer!!)
//            secondaryDisplayPresentation?.show()
//        }
//    }

    companion object {
        init {
            // 加载你的 JNI 库
            System.loadLibrary("myappTest")
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.activity_main)
        hideStatusBar(window, window.decorView)


        surfaceView = findViewById(R.id.surfaceView)
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
        // 1. 获取 Context 对象 (在 Activity 中，可以直接用 'this' 或 'applicationContext')
        //val context: Context = this

        // 2. 获取 Display 对象 (需要通过 DisplayManager)
//        val displayManager = getSystemService<DisplayManager>()
//
//        val defaultDisplay: Display = displayManager?.getDisplay(android.view.Display.DEFAULT_DISPLAY)
//            ?: throw IllegalStateException("Cannot get default display")


        val displayManager = getSystemService(Context.DISPLAY_SERVICE) as DisplayManager
        if(displayManager.displays.count()>1){

            val displays = displayManager.displays[1]

            myPresentation = MyPresentation(applicationContext,displays)
//
            myPresentation.show()
        }


    }


    override fun onPause() {
        super.onPause()
        // 推荐的做法是在 onPause/onResume 中管理渲染状态，这里保持简单，只在 Surface 生命周期中管理
    }
}