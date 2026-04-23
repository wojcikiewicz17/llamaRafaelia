# assembler

`rmrCti/` foi mantido intacto.

Este diretório contém:

1. Programas em assembly syscall-only para `x86_64`, `arm64` e `armv7`.
2. Exportador dos programas C em `rmrCti/` para assembly `x86_64` (`.s`) em lote.

## Programas assembly nativos

- `src/hello_x86_64.S`
- `src/hello_arm64.S`
- `src/hello_armv7.S`

## Exportar `rmrCti/*.c` para assembly

```bash
cd assembler
./export-rmrcti-asm.sh
```

Saída:

- `assembler/rmrCti_x86_64/*.s`
- `assembler/rmrCti_x86_64/compiled.txt`
- `assembler/rmrCti_x86_64/failed.txt`

## Build local

```bash
cd assembler
make all
```

`make all` executa:

- exportação de `rmrCti` para assembly x86_64;
- build dos binários `hello_*` para `x86_64`, `arm64`, `armv7`.

## Execução local

```bash
cd assembler
make run-x86
```

## CI

Workflow: `.github/workflows/assembler-build.yml`

- exporta `rmrCti` para assembly x86_64;
- compila os 3 binários;
- publica `assembler-dist` e `assembler-rmrcti-x86_64` em artifacts do GitHub Actions.
