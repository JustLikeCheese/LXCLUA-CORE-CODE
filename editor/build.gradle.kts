plugins {
    id("com.android.library")
    id("org.jetbrains.kotlin.android")
}

object Versions {
    const val androidxAnnotation = "1.8.2"
    const val versionName = "1.0.0"
}

group = "io.github.Rosemoe.sora-editor"
version = Versions.versionName

android {
    namespace = "io.github.rosemoe.sora"
    compileSdk = 35

    defaultConfig {
        minSdk = 21
        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
        consumerProguardFiles("consumer-rules.pro")
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
    

    // 使用新的Kotlin compilerOptions API
    tasks.withType<org.jetbrains.kotlin.gradle.tasks.KotlinCompile>().configureEach {
        compilerOptions {
            jvmTarget.set(org.jetbrains.kotlin.gradle.dsl.JvmTarget.JVM_17)
        }
    }
}

dependencies {

    //Material
    implementation("com.google.android.material:material:1.14.0-alpha08")

    // AndroidX
    api("androidx.annotation:annotation:${Versions.androidxAnnotation}")
    
    implementation(project(":material-ui"))
    
    implementation("androidx.collection:collection:1.6.0-alpha01")
    
    // Kotlin
    implementation("org.jetbrains.kotlin:kotlin-stdlib:2.3.0")
    
    // Testing
    testImplementation("junit:junit:4.13.2")
    testImplementation("com.google.truth:truth:1.4.4")
    testImplementation("org.robolectric:robolectric:4.13")
    
    // Android Testing
    androidTestImplementation("androidx.test.ext:junit:1.2.1")
    androidTestImplementation("androidx.test.espresso:espresso-core:3.6.1")
}