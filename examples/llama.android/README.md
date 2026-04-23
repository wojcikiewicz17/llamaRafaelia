# llama.android — especificação mínima executável de build/release

Este documento define o caminho **local + CI** para gerar artefatos Android com JNI/CMake/NDK, incluindo trilha de validação interna (**unsigned**) e trilha oficial de publicação (**signed**).

## 1) Pré-requisitos fixos

> Versões travadas para evitar drift entre máquina local e CI.

- **JDK:** 17
- **Android Gradle Plugin (AGP):** 8.2.0 (definido em `build.gradle.kts` raiz do exemplo)
- **Gradle Wrapper:** usar sempre `./gradlew` do projeto (não usar Gradle global)
- **Android SDK (mínimo):**
  - `platforms;android-34`
  - `build-tools;34.0.0`
  - `platform-tools`
  - `cmdline-tools;latest`
- **NDK:** `26.1.10909125` (fixar em `local.properties`)
- **CMake:** 3.22.1 (coerente com `externalNativeBuild.cmake.version`)

### Bootstrap recomendado (Linux/macOS)

No diretório `examples/llama.android`:

```bash
export ANDROID_SDK_ROOT="$HOME/Android/Sdk"

yes | "$ANDROID_SDK_ROOT/cmdline-tools/latest/bin/sdkmanager" --licenses
"$ANDROID_SDK_ROOT/cmdline-tools/latest/bin/sdkmanager" \
  "platform-tools" \
  "platforms;android-34" \
  "build-tools;34.0.0" \
  "cmake;3.22.1" \
  "ndk;26.1.10909125"

cat > local.properties <<LOCALPROPS
sdk.dir=${ANDROID_SDK_ROOT}
ndk.dir=${ANDROID_SDK_ROOT}/ndk/26.1.10909125
cmake.dir=${ANDROID_SDK_ROOT}/cmake/3.22.1
LOCALPROPS
```

## 2) Comandos de build local (debug/release)

Executar sempre em `examples/llama.android`.

### Build debug

```bash
./gradlew clean :app:assembleDebug --no-daemon
```

Saída principal:

- `app/build/outputs/apk/debug/app-debug.apk`

### Build release (unsigned)

```bash
./gradlew clean :app:assembleRelease :app:bundleRelease --no-daemon
```

Saídas principais:

- APK unsigned: `app/build/outputs/apk/release/app-release-unsigned.apk`
- Bundle release (AAB): `app/build/outputs/bundle/release/app-release.aab`

## 3) Fluxo explícito: unsigned (interna) e signed (oficial)

### 3.1 Validação interna (unsigned)

Objetivo: QA interna, validação JNI/ABI e smoke tests sem uso de chave oficial.

```bash
./gradlew clean :app:assembleRelease --no-daemon
```

Artefato:

- `app/build/outputs/apk/release/app-release-unsigned.apk`

Validação:

```bash
# verifica que está unsigned (esperado)
$ANDROID_SDK_ROOT/build-tools/34.0.0/apksigner verify --verbose app/build/outputs/apk/release/app-release-unsigned.apk
# deve indicar que não há assinaturas válidas
```

### 3.2 Release oficial (signed)

> Não substituir fluxo oficial por unsigned.

1. Gerar unsigned:

```bash
./gradlew clean :app:assembleRelease --no-daemon
```

2. Assinar APK com keystore oficial:

```bash
export KEYSTORE_PATH=/caminho/llama-release.jks
export KEY_ALIAS=llama_release
export KEYSTORE_PASS='***'
export KEY_PASS='***'

cp app/build/outputs/apk/release/app-release-unsigned.apk \
   app/build/outputs/apk/release/app-release-signed.apk

$ANDROID_SDK_ROOT/build-tools/34.0.0/apksigner sign \
  --ks "$KEYSTORE_PATH" \
  --ks-key-alias "$KEY_ALIAS" \
  --ks-pass env:KEYSTORE_PASS \
  --key-pass env:KEY_PASS \
  --out app/build/outputs/apk/release/app-release-signed.apk \
  app/build/outputs/apk/release/app-release-unsigned.apk
```

3. Verificar assinatura:

```bash
$ANDROID_SDK_ROOT/build-tools/34.0.0/apksigner verify --verbose --print-certs \
  app/build/outputs/apk/release/app-release-signed.apk
```

## 4) Convenção de nomes e paths de artefatos

- Debug APK:
  - `app/build/outputs/apk/debug/app-debug.apk`
- Release APK unsigned:
  - `app/build/outputs/apk/release/app-release-unsigned.apk`
- Release APK signed (saída pós-apksigner):
  - `app/build/outputs/apk/release/app-release-signed.apk`
- Release AAB:
  - `app/build/outputs/bundle/release/app-release.aab`
- Metadados de output AGP:
  - `app/build/outputs/**/output-metadata.json`

## 5) ABIs suportadas e validação arm32/arm64

Sem `abiFilters` configurado, o build JNI usa os ABIs padrão suportados pelo plugin/NDK.

Para forçar build multi-ABI típico de validação (incluindo arm32 + arm64), configurar no módulo nativo (`llama/build.gradle.kts`):

```kotlin
android {
    defaultConfig {
        ndk {
            abiFilters += listOf("armeabi-v7a", "arm64-v8a")
        }
    }
}
```

### Validar presença de libs JNI no APK

```bash
unzip -l app/build/outputs/apk/release/app-release-unsigned.apk | grep '^.*lib/armeabi-v7a/\|^.*lib/arm64-v8a/'
```

Esperado: entradas para as duas pastas ABI.

### Validar ABI de dispositivo conectado

```bash
adb shell getprop ro.product.cpu.abi
adb shell getprop ro.product.cpu.abilist
```

## 6) Mapeamento local ↔ CI (`.github/workflows/build.yml`)

### Job `android-build`

- CI executa:

```bash
cd examples/llama.android
./gradlew build --no-daemon
```

- Equivalência local direta:

```bash
cd examples/llama.android
./gradlew build --no-daemon
```

### Job `android-ndk-build`

- CI cobre build nativo com CMake/NDK em matriz (ex.: `arm64-cpu`, `arm64-snapdragon`) usando toolchain Android e presets/defines específicos.
- Equivalência local (nativo puro, fora AGP), exemplo arm64 CPU:

```bash
cmake -B build-android-arm64 \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-31 \
  -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake" \
  -DGGML_NATIVE=OFF \
  -DGGML_CPU_ARM_ARCH=armv8.5-a+fp16+i8mm \
  -DLLAMA_CURL=OFF \
  -DGGML_OPENMP=OFF \
  -G Ninja
cmake --build build-android-arm64 --config Release -j
```

## 7) Troubleshooting (NDK/CMake/JNI)

### Erro: `NDK not configured` / `No version of NDK matched`

- Confirmar `local.properties` com `ndk.dir` válido.
- Confirmar instalação da versão fixa via `sdkmanager`.
- Limpar caches e reconstruir:

```bash
./gradlew --stop
rm -rf ~/.gradle/caches
./gradlew clean :app:assembleDebug --no-daemon
```

### Erro: `CMake 3.22.1 was not found`

- Instalar exatamente `cmake;3.22.1` no SDK.
- Garantir `cmake.dir` em `local.properties`.

### Erro JNI em runtime: `java.lang.UnsatisfiedLinkError`

- Verificar nome da lib carregada em Kotlin (`System.loadLibrary("llama-android")`) vs saída de CMake.
- Verificar presença da ABI da device dentro do APK (`unzip -l ... | grep lib/<abi>`).
- Verificar se build foi feito com ABI compatível com dispositivo.

### Erro de link nativo (`undefined reference`, `ld.lld`)

- Confirmar que as flags CMake/NDK da configuração atual batem com o job alvo.
- Limpar build nativo incremental:

```bash
rm -rf app/.cxx llama/.cxx */build
./gradlew clean :app:assembleRelease --no-daemon
```

### Erro de assinatura APK

- Confirmar alias e senhas da keystore.
- Validar com `apksigner verify --print-certs`.
- Nunca distribuir `app-release-unsigned.apk` como release oficial.
