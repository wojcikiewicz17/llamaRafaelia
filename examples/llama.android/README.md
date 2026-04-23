# llama.android

## Assinatura de release (oficial) + trilha unsigned

O módulo `app` possui duas trilhas de release:

- `release`: trilha oficial, assinada **somente** quando as credenciais de assinatura oficial existem.
- `releaseUnsigned`: trilha de validação interna, sem assinatura oficial, separada da release oficial.

### Credenciais aceitas (Gradle property ou variável de ambiente)

- `LLAMA_ANDROID_RELEASE_STORE_FILE`
- `LLAMA_ANDROID_RELEASE_STORE_PASSWORD`
- `LLAMA_ANDROID_RELEASE_KEY_ALIAS`
- `LLAMA_ANDROID_RELEASE_KEY_PASSWORD`

## Configuração local (sem hardcode)

Você pode definir no `~/.gradle/gradle.properties` (recomendado) ou em `examples/llama.android/gradle.properties` local não versionado:

```properties
LLAMA_ANDROID_RELEASE_STORE_FILE=/abs/path/release.keystore
LLAMA_ANDROID_RELEASE_STORE_PASSWORD=***
LLAMA_ANDROID_RELEASE_KEY_ALIAS=***
LLAMA_ANDROID_RELEASE_KEY_PASSWORD=***
```

Ou exportar no shell:

```bash
export LLAMA_ANDROID_RELEASE_STORE_FILE=/abs/path/release.keystore
export LLAMA_ANDROID_RELEASE_STORE_PASSWORD=***
export LLAMA_ANDROID_RELEASE_KEY_ALIAS=***
export LLAMA_ANDROID_RELEASE_KEY_PASSWORD=***
```

## CI (GitHub Actions) sem hardcode

Mapeie secrets do repositório para variáveis de ambiente do job:

```yaml
env:
  LLAMA_ANDROID_RELEASE_STORE_FILE: ${{ secrets.LLAMA_ANDROID_RELEASE_STORE_FILE }}
  LLAMA_ANDROID_RELEASE_STORE_PASSWORD: ${{ secrets.LLAMA_ANDROID_RELEASE_STORE_PASSWORD }}
  LLAMA_ANDROID_RELEASE_KEY_ALIAS: ${{ secrets.LLAMA_ANDROID_RELEASE_KEY_ALIAS }}
  LLAMA_ANDROID_RELEASE_KEY_PASSWORD: ${{ secrets.LLAMA_ANDROID_RELEASE_KEY_PASSWORD }}
```

## Comandos de build

Release oficial (falha cedo se credenciais não existirem):

```bash
./gradlew :app:assembleRelease
```

Release unsigned para validação interna:

```bash
./gradlew :app:assembleReleaseUnsigned
```
