plugins {
    id("com.android.library")
    //alias(libs.plugins.android.library)
    alias(libs.plugins.kotlin.android)
}
android {
    namespace = "com.example.Application"
    compileSdk = 33

    defaultConfig {
        //applicationId = "com.example.SecondRendering"
        minSdk = 29
        //targetSdk = 33
        //versionCode = 1
        //versionName = "1.0"

//        // 这里很关键：告诉Gradle要编译本地C++库
//        externalNativeBuild {
//            cmake {
//                cppFlags += listOf("-std=c++17", "-Wall")
//            }
//        }

        ndk {
            abiFilters += listOf("arm64-v8a", "armeabi-v7a")
        }

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }
    kotlinOptions {
        jvmTarget = "11"
    }

    // 启用CMake构建
    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }

    buildFeatures {
        viewBinding = true
    }
}

configurations.all {
    resolutionStrategy {
        force("androidx.activity:activity:1.7.2")
        force("androidx.activity:activity-ktx:1.7.2")
    }
}
dependencies {
    compileOnly(files("libs/classes.jar")) // <--- 添加这行
    implementation("androidx.activity:activity:1.7.2")
    implementation ("com.google.android.material:material:1.8.0")
    implementation(libs.androidx.core.ktx)
    implementation(libs.androidx.appcompat)
    implementation(libs.material)
    implementation(libs.androidx.constraintlayout)
    testImplementation(libs.junit)
    androidTestImplementation(libs.androidx.junit)
    androidTestImplementation(libs.androidx.espresso.core)
}
//afterEvaluate {
//    tasks.named("assembleRelease").configure {
//        doLast {
//            copy {
//                from("build/outputs/aar/MyPlugin-release.aar")
//                into("D:/UnityAARs/")  // 👈 你指定的目标路径
//            }
//        }
//    }
//}


tasks.register<Copy>("copyAar") {
    val sourceAarFileName = "app-release.aar"

    // 1. 查找源文件：默认的输出目录
    from(fileTree(mapOf(
        "dir" to "$buildDir/outputs/aar",
        "include" to sourceAarFileName
    )))

    // 2. 使用 into() 设置目标目录
    // rootDir 是对项目根目录的引用
    into("D:\\UNITY\\TESTShader\\Screen\\Assets\\Plugins\\Android")

    // 确保目标目录存在 (使用 File.mkdirs() 或 Kotlin 的 .mkdirs())
//    doFirst {
//        rootDir.resolve("artifacts").mkdirs()
//    }
}

// 确保 copyAar 任务在 assembleRelease 任务完成后运行
tasks.named("assemble") {
    finalizedBy("copyAar")
}