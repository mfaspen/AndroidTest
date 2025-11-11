package com.example.Application

import android.app.Presentation
import android.content.Context
import android.os.Bundle
import android.view.Display
import android.view.SurfaceHolder
import android.view.SurfaceView

class MyPresentation (outerContext: Context, display: Display): Presentation(outerContext,display){

    private var secondHandle = NativeRenderer()

    override fun onCreate(savedInstanceState: Bundle?){

        super.onCreate(savedInstanceState)

        val surfaceView = SurfaceView(context)

        setContentView(surfaceView)

        surfaceView.holder.addCallback(object : SurfaceHolder.Callback{

            override fun surfaceCreated(holder: SurfaceHolder){
                secondHandle.primaryRender(holder.surface)
            }

            override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int){

            }

            override fun surfaceDestroyed(holder: SurfaceHolder){

            }

        })

    }




}