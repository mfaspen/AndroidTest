# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

# Declares the project name. The project name can be accessed via ${ PROJECT_NAME},
# Since this is the top level CMakeLists.txt, the project name is also accessible
# with ${CMAKE_PROJECT_NAME} (both CMake variables are in-sync within the top level
# build script scope).
project(SecondRendering)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
#
# In this top level CMakeLists.txt, ${CMAKE_PROJECT_NAME} is used to define
# the target library name; in the sub-module's CMakeLists.txt, ${PROJECT_NAME}
# is preferred for the same purpose.
#
# In order to load a library into your app from Java/Kotlin, you must call
# System.loadLibrary() and pass the name of the library defined here;
# for GameActivity/NativeActivity derived applications, the same library name must be
# used in the AndroidManifest.xml file.
add_library(${PROJECT_NAME} SHARED
        # List C/C++ source files with relative paths to this CMakeLists.txt.
        NaitveRender.cpp interface.cpp getSlam.cpp egl/GLRenderer.cpp egl/PrimaryRenderer.cpp egl/SecondaryRenderer.cpp egl/slam.cpp)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/
)

find_library( log-lib log )
find_library( android-lib android )
find_library( egl-lib EGL )
find_library( gles-lib GLESv3 )

target_include_directories(${PROJECT_NAME}
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include/
        ${CMAKE_CURRENT_SOURCE_DIR}/include2/
)

# 定义一个全局变量来存储所有导入的库
set(ALL_IMPORTED_LIBS "" CACHE INTERNAL "List of all imported libraries")

# 批量导入函数
#function(import_prebuilt_libs lib_dir)
#    file(GLOB lib_files "${lib_dir}/*.so")
#
#    foreach(lib_file ${lib_files})
#        get_filename_component(lib_name ${lib_file} NAME_WE)
#        string(REGEX REPLACE "^lib" "" lib_name ${lib_name})
#
#        message(STATUS "导入预编译库: ${lib_name} -> ${lib_file}")
#
#        add_library(${lib_name} SHARED IMPORTED)
#        set_target_properties(${lib_name} PROPERTIES
#            IMPORTED_LOCATION ${lib_file}
#        )
#
#        # 将库名称添加到全局列表
#        set(ALL_IMPORTED_LIBS ${ALL_IMPORTED_LIBS} ${lib_name} CACHE INTERNAL "List of all imported libraries")
#    endforeach()
#endfunction()

# 使用函数导入所有库
#import_prebuilt_libs(${CMAKE_CURRENT_SOURCE_DIR}/libs/${ANDROID_ABI})

# 导入多个第三方库
add_library(xv_sdk SHARED IMPORTED)
set_target_properties(xv_sdk PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../../../libs/${ANDROID_ABI}/libxslam-xv-sdk.so
)

# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(${PROJECT_NAME}
        # List libraries link to the target library
        ${log-lib}
        ${android-lib}
        ${egl-lib}
        ${gles-lib}
        xv_sdk
)
