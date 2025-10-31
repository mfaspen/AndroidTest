package com.example.Application

import android.app.PendingIntent
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.hardware.usb.UsbDevice
import android.hardware.usb.UsbDeviceConnection
import android.hardware.usb.UsbManager
import android.os.Build
import androidx.appcompat.app.AppCompatActivity

private const val ACTION_USB_PERMISSION = "com.example.USB_PERMISSION"

object DeviceInit
{

    private const val ACTION_USB_PERMISSION = "com.your.sdk.USB_PERMISSION"

    private lateinit var usbManager: UsbManager
    private lateinit var permissionIntent: PendingIntent
    private var callback: ((UsbResult) -> Unit)? = null

    fun init(context: Context) {
        usbManager = context.getSystemService(Context.USB_SERVICE) as UsbManager

        permissionIntent = PendingIntent.getBroadcast(
            context, 0, Intent(ACTION_USB_PERMISSION),
            PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_MUTABLE
        )

        val filter = IntentFilter(ACTION_USB_PERMISSION)
        context.registerReceiver(usbReceiver, filter)
    }

    fun requestPermission(cb: (UsbResult) -> Unit) {
        callback = cb

        val device = usbManager.deviceList.values.firstOrNull()
        if (device == null) {
            cb(UsbResult(false, "未发现 USB 设备"))
            return
        }

        if (usbManager.hasPermission(device)) {
            openDevice(device)
        } else {
            usbManager.requestPermission(device, permissionIntent)
        }
    }

    private val usbReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context?, intent: Intent?) {
            if (intent?.action != ACTION_USB_PERMISSION) return

            val device = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                intent.getParcelableExtra(UsbManager.EXTRA_DEVICE, UsbDevice::class.java)
            } else {
                @Suppress("DEPRECATION")
                intent.getParcelableExtra(UsbManager.EXTRA_DEVICE)
            }

            if (device != null && intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                openDevice(device)
            } else {
                callback?.invoke(UsbResult(false, "USB 权限拒绝"))
            }
        }
    }

    private fun openDevice(device: UsbDevice) {
        val connection = usbManager.openDevice(device)
        if (connection != null) {
            callback?.invoke(UsbResult(true, "USB 打开成功", connection, device))
        } else {
            callback?.invoke(UsbResult(false, "USB 打开失败"))
        }
    }









}

data class UsbResult(
    val success: Boolean,
    val message: String,
    val connection: UsbDeviceConnection? = null,
    val device: UsbDevice? = null
)