package com.example.Application

import android.app.Activity
import android.app.PendingIntent
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.hardware.usb.UsbDevice
import android.hardware.usb.UsbDeviceConnection
import android.hardware.usb.UsbManager
import android.os.Build
import android.os.Message
import android.util.Log
import org.xvisio.xvsdk.UsbUtilities.grantUsbPermission
import org.xvisio.xvsdk.UsbUtilities.hasUsbPermission


private const val ACTION_USB_PERMISSION = "com.example.Application.USB_PERMISSION"

object DeviceInit
{

    private var nativeRenderer = NativeRenderer()
    private const val TAG: String = "XVisio DeviceWatcher"
    private val mDescriptors: HashMap<String?, UsbDesc?> = LinkedHashMap<String?, UsbDesc?>()
    private lateinit var usbManager: UsbManager
    private lateinit var permissionIntent: PendingIntent
    private var callback: ((UsbResult) -> Unit)? = null



    fun init(context: Context) {
        usbManager = context.getSystemService(Context.USB_SERVICE) as UsbManager

//        permissionIntent = PendingIntent.getBroadcast(
//            context, 0, Intent(ACTION_USB_PERMISSION),
//            PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_MUTABLE
//        )

        // Android 13 (API 33) 必须这样写
        val flags = PendingIntent.FLAG_IMMUTABLE or PendingIntent.FLAG_UPDATE_CURRENT

        permissionIntent = PendingIntent.getBroadcast(
            context,
            0,
            Intent(ACTION_USB_PERMISSION),
            flags
        )


        val filter = IntentFilter(ACTION_USB_PERMISSION)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            // 针对 Android 13 (API 33) 及更高版本

            // 确保您的 Context 是 Activity，否则需要第三个参数
            if (context is Activity) {
                // 如果在 Activity 中注册，通常只需要前两个参数（系统会自动处理导出状态）
                context.registerReceiver(usbReceiver, filter)
            } else {
                // 如果在 Application Context 中注册，则必须使用 RECEIVER_NOT_EXPORTED
                context.registerReceiver(usbReceiver, filter, Context.RECEIVER_NOT_EXPORTED)
            }

        } else {
            // 低于 API 33 的所有版本，只需使用两个参数
            context.registerReceiver(usbReceiver, filter)
        }


    }

    private fun onDeviceAttach(context: Context) {
        val msg = Message.obtain()
        if (!hasUsbPermission(context)) {
            grantUsbPermission(context)
            return
        }
    }

    private fun addDevice(device: UsbDevice?,context: Context) {
        if (device == null) return

        val usbManager = context.getSystemService(Context.USB_SERVICE) as UsbManager
        val conn = usbManager.openDevice(device)
        if (conn == null) return
        val desc = UsbDesc(device.getDeviceName(), conn.getFileDescriptor(), conn)
        Log.d(TAG, "Adding device: " + desc.name)
        mDescriptors.put(device.getDeviceName(), desc)
        nativeRenderer.addUsbDevice(desc.name, desc.descriptor)


        Log.d(TAG, "Device: " + desc.name + " added successfully")
    }


    fun requestPermission(cb: (UsbResult) -> Unit,context: Context) {

        callback = cb
        
        val deviceList = usbManager.deviceList.values

        Log.e("USB_CHECK", "Found ${deviceList.size} devices.")
        for (device in deviceList) {
            Log.e("USB_CHECK", "Device: ${device.deviceName}, VID: ${device.vendorId}, PID: ${device.productId}")
        }

        val devicesMap = usbManager.deviceList
        val xvisioDevices: MutableList<String?> = ArrayList<String?>()
        for (entry in devicesMap.entries) {
            val usbDevice = entry.value

            if (usbDevice.vendorId == 0x040e) xvisioDevices.add(entry.key)


        }
        val iter: MutableIterator<MutableMap.MutableEntry<String?, UsbDesc?>> =
            mDescriptors.entries.iterator()

        while (iter.hasNext()) {
            val entry = iter.next()
            if (!xvisioDevices.contains(entry.key)) {
                //removeDevice(entry.value)
                iter.remove()
            }
        }

        for (name in xvisioDevices) {

            val device = usbManager.deviceList.get(name)
            //val device = usbManager.deviceList.values.firstOrNull()
            if (device == null) {
                cb(UsbResult(false, "USB_CHECK 未发现 USB 设备"))
                return
            }
            Log.e("USB_CHECK", "hasPermission1 = ${usbManager.hasPermission(device)} Device: ${device.deviceName}, VID: ${device.vendorId}, PID: ${device.productId}" )

            for (i in 0..<device.interfaceCount) {
                val inf = device.getInterface(i)
                Log.d(
                    "USB", "Interface " + i + ": class=" + inf.interfaceClass +
                            ", subclass=" + inf.interfaceSubclass +
                            ", protocol=" + inf.interfaceProtocol
                )
            }

            if (usbManager.hasPermission(device)) {
                openDevice(device)
            } else {
                usbManager.requestPermission(device, permissionIntent)
            }

        }



    }
    
    fun getFd(context: Context){


        onDeviceAttach(context)

        val devicesMap = usbManager.deviceList
        val xvisioDevices: MutableList<String?> = ArrayList<String?>()
        for (entry in devicesMap.entries) {
            val usbDevice = entry.value

            if (usbDevice.vendorId == 0x040e) xvisioDevices.add(entry.key)


        }
        val iter: MutableIterator<MutableMap.MutableEntry<String?, UsbDesc?>> =
            mDescriptors.entries.iterator()

        while (iter.hasNext()) {
            val entry = iter.next()
            if (!xvisioDevices.contains(entry.key)) {
                //removeDevice(entry.value)
                iter.remove()
            }
        }
        Log.e("USB_CHECK", "looking xv")
        for (name in xvisioDevices) {
            Log.e("USB_CHECK", "name = $name" )
            Log.e("USB_CHECK", "mDescriptors = ${mDescriptors.containsKey(name)}" )
            if (!mDescriptors.containsKey(name)) {
                val device = devicesMap.get(name)

                Log.e("USB_CHECK", "hasPermission = ${usbManager.hasPermission(devicesMap.get(name))}" )


                val connection: UsbDeviceConnection? = usbManager.openDevice(usbManager.deviceList.get(name))

                //val conn = usbManager.openDevice(devicesMap.get(name))

                //val connection = UsbDeviceConnection(devicesMap.get(name))


                if (connection != null) {


                    Log.e("USB_CHECK", "1" )
                    val fd: Int = connection.fileDescriptor

                    Log.e("USB_CHECK", "fd = $fd" )
                    addDevice(devicesMap.get(name),context)

                }else{

                    Log.e("USB_CHECK", "0" )
                }

            }
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