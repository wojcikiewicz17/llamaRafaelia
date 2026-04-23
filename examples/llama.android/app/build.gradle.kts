import org.gradle.api.GradleException

plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

fun secret(name: String): String? {
    return providers.gradleProperty(name).orNull
        ?: providers.environmentVariable(name).orNull
}

val releaseStoreFile = secret("LLAMA_ANDROID_RELEASE_STORE_FILE")
val releaseStorePassword = secret("LLAMA_ANDROID_RELEASE_STORE_PASSWORD")
val releaseKeyAlias = secret("LLAMA_ANDROID_RELEASE_KEY_ALIAS")
val releaseKeyPassword = secret("LLAMA_ANDROID_RELEASE_KEY_PASSWORD")

val hasOfficialReleaseSigningCreds = listOf(
    releaseStoreFile,
    releaseStorePassword,
    releaseKeyAlias,
    releaseKeyPassword,
).all { !it.isNullOrBlank() }

val requestedOfficialRelease = gradle.startParameter.taskNames.any { taskName ->
    val normalized = taskName.lowercase()
    normalized.contains("release") && !normalized.contains("unsigned")
}

if (requestedOfficialRelease && !hasOfficialReleaseSigningCreds) {
    throw GradleException(
        "Official release signing credentials are missing. Provide LLAMA_ANDROID_RELEASE_STORE_FILE, " +
            "LLAMA_ANDROID_RELEASE_STORE_PASSWORD, LLAMA_ANDROID_RELEASE_KEY_ALIAS, " +
            "LLAMA_ANDROID_RELEASE_KEY_PASSWORD via Gradle properties or environment variables. " +
            "Use :app:assembleReleaseUnsigned for internal unsigned validation builds."
    )
}

android {
    namespace = "com.example.llama"
    compileSdk = 34

    defaultConfig {
        applicationId = "com.example.llama"
        minSdk = 33
        targetSdk = 34
        versionCode = 1
        versionName = "1.0"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
        vectorDrawables {
            useSupportLibrary = true
        }
    }

    signingConfigs {
        if (hasOfficialReleaseSigningCreds) {
            create("releaseOfficial") {
                storeFile = file(requireNotNull(releaseStoreFile))
                storePassword = requireNotNull(releaseStorePassword)
                keyAlias = requireNotNull(releaseKeyAlias)
                keyPassword = requireNotNull(releaseKeyPassword)
            }
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
            if (hasOfficialReleaseSigningCreds) {
                signingConfig = signingConfigs.getByName("releaseOfficial")
            }
        }

        create("releaseUnsigned") {
            initWith(getByName("release"))
            signingConfig = null
            matchingFallbacks += listOf("release")
        }
    }
    splits {
        abi {
            isEnable = true
            reset()
            include("armeabi-v7a", "arm64-v8a")
            isUniversalApk = false
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }
    kotlinOptions {
        jvmTarget = "1.8"
    }
    buildFeatures {
        compose = true
    }
    composeOptions {
        kotlinCompilerExtensionVersion = "1.5.1"
    }
}

dependencies {

    implementation("androidx.core:core-ktx:1.12.0")
    implementation("androidx.lifecycle:lifecycle-runtime-ktx:2.6.2")
    implementation("androidx.activity:activity-compose:1.8.2")
    implementation(platform("androidx.compose:compose-bom:2023.08.00"))
    implementation("androidx.compose.ui:ui")
    implementation("androidx.compose.ui:ui-graphics")
    implementation("androidx.compose.ui:ui-tooling-preview")
    implementation("androidx.compose.material3:material3")
    implementation(project(":llama"))
    testImplementation("junit:junit:4.13.2")
    androidTestImplementation("androidx.test.ext:junit:1.1.5")
    androidTestImplementation("androidx.test.espresso:espresso-core:3.5.1")
    androidTestImplementation(platform("androidx.compose:compose-bom:2023.08.00"))
    androidTestImplementation("androidx.compose.ui:ui-test-junit4")
    debugImplementation("androidx.compose.ui:ui-tooling")
    debugImplementation("androidx.compose.ui:ui-test-manifest")
}
