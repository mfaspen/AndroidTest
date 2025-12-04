plugins {
    id("com.android.library") version "7.0.3"
    id("org.jetbrains.kotlin.android") version "1.5.31"
}

android {
    namespace = "com.example.Application"
    compileSdk = 29

    defaultConfig {
        minSdk = 29

        ndk {
            abiFilters.add("arm64-v8a")
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
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
    kotlinOptions {
        jvmTarget = "17"
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }

    buildFeatures {
        viewBinding = true
    }

    sourceSets {
        getByName("main") {
            jniLibs.srcDir("libs")
        }
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
    implementation("com.google.android.material:material:1.6.1")
    implementation("androidx.core:core-ktx:1.10.1")
    implementation("androidx.appcompat:appcompat:1.6.1")
    implementation("androidx.constraintlayout:constraintlayout:2.1.4")
    testImplementation("junit:junit:4.13.2")
    androidTestImplementation("androidx.test.ext:junit:1.1.5")
    androidTestImplementation("androidx.test.espresso:espresso-core:3.5.1")
}



tasks.register<Copy>("copyAar") {
    val sourceAarFileName = "app-release.aar"

    // 1. 查找源文件：默认的输出目录
    from(fileTree(mapOf(
        "dir" to "$build/Diroutputs/aar",
        "include" to sourceAarFileName
    )))

    into("D:\\UNITY\\TESTShader\\Screen\\Assets\\Plugins\\Android")


}

// 确保 copyAar 任务在 assembleRelease 任务完成后运行
tasks.named("assemble") {
    finalizedBy("copyAar")
}