# llama.android build notes

## ABIs supported

The Android native + APK build is explicitly pinned to:

- `armeabi-v7a`
- `arm64-v8a`

Source of truth:

- NDK ABI filters in `llama/build.gradle.kts`
- ABI guard in `llama/src/main/cpp/CMakeLists.txt`
- APK splits in `app/build.gradle.kts`

## Expected output directories

After running `./gradlew :app:assembleRelease :app:assembleDebug` from `examples/llama.android`:

- Native JNI outputs:
  - `llama/build/intermediates/merged_native_libs/release/mergeReleaseNativeLibs/out/lib/armeabi-v7a/libllama-android.so`
  - `llama/build/intermediates/merged_native_libs/release/mergeReleaseNativeLibs/out/lib/arm64-v8a/libllama-android.so`
- Unsigned release APKs per ABI:
  - `app/build/outputs/apk/release/app-armeabi-v7a-release-unsigned.apk`
  - `app/build/outputs/apk/release/app-arm64-v8a-release-unsigned.apk`
- Signed debug APKs per ABI:
  - `app/build/outputs/apk/debug/app-armeabi-v7a-debug.apk`
  - `app/build/outputs/apk/debug/app-arm64-v8a-debug.apk`

## CI

Workflow: `.github/workflows/android-abi.yml`

The workflow validates ABI-specific native outputs and APK outputs, then uploads artifacts grouped by ABI and signature mode.

`*-internal-signed.apk` files are generated only for CI/internal validation and are not an official production release signature path.
