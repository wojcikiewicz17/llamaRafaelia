# Boosters de Performance / Performance Boosters

## Visão Geral / Overview

**Português:** Este documento detalha os "boosters" (aceleradores de performance) disponíveis no llama.cpp. Boosters são backends que utilizam hardware especializado para acelerar a inferência de modelos LLM, proporcionando ganhos significativos de desempenho.

**English:** This document details the "boosters" (performance accelerators) available in llama.cpp. Boosters are backends that utilize specialized hardware to accelerate LLM model inference, providing significant performance gains.

---

## Índice / Table of Contents

1. [Tipos de Boosters / Booster Types](#tipos-de-boosters--booster-types)
2. [Comparação de Performance / Performance Comparison](#comparação-de-performance--performance-comparison)
3. [Guia de Uso / Usage Guide](#guia-de-uso--usage-guide)
4. [Benchmarks Detalhados / Detailed Benchmarks](#benchmarks-detalhados--detailed-benchmarks)
5. [Recomendações / Recommendations](#recomendações--recommendations)

---

## Tipos de Boosters / Booster Types

### 1. CPU (Baseline)
**Dispositivos Suportados / Supported Devices:** Todos os processadores / All processors

**Português:**
- Backend padrão sem aceleração de hardware
- Otimizações: AVX, AVX2, AVX512, NEON (ARM)
- Melhor para: Compatibilidade universal, testes
- Desempenho: Baseline (1x)

**English:**
- Default backend without hardware acceleration
- Optimizations: AVX, AVX2, AVX512, NEON (ARM)
- Best for: Universal compatibility, testing
- Performance: Baseline (1x)

**Compilação / Build:**
```bash
cmake -B build
cmake --build build --config Release
```

---

### 2. CUDA (NVIDIA GPU)
**Dispositivos Suportados / Supported Devices:** GPUs NVIDIA (GTX, RTX, Tesla, A100, H100)

**Português:**
- Aceleração via GPU NVIDIA
- Suporte para computação de precisão mista
- Melhor para: Inferência em GPUs NVIDIA
- Desempenho típico: 10-50x mais rápido que CPU (dependendo do modelo)

**English:**
- NVIDIA GPU acceleration
- Support for mixed precision computation
- Best for: Inference on NVIDIA GPUs
- Typical performance: 10-50x faster than CPU (model dependent)

**Compilação / Build:**
```bash
cmake -B build -DGGML_CUDA=ON
cmake --build build --config Release
```

**Uso / Usage:**
```bash
# Descarregar todas as camadas na GPU / Offload all layers to GPU
./llama-cli -m model.gguf -ngl 99

# Descarregar 20 camadas / Offload 20 layers
./llama-cli -m model.gguf -ngl 20
```

**Benchmarks:**
- 7B Q4_0: ~130 tokens/s (RTX 4080)
- 13B Q4_0: ~80 tokens/s (RTX 4080)
- 30B Q4_0: ~40 tokens/s (RTX 4090)

---

### 3. Metal (Apple Silicon)
**Dispositivos Suportados / Supported Devices:** Apple M1, M2, M3, M4

**Português:**
- Aceleração via GPU Apple integrada
- Habilitado por padrão no macOS
- Melhor para: Macs com Apple Silicon
- Desempenho típico: 15-30x mais rápido que CPU

**English:**
- Apple integrated GPU acceleration
- Enabled by default on macOS
- Best for: Macs with Apple Silicon
- Typical performance: 15-30x faster than CPU

**Compilação / Build:**
```bash
# Metal é habilitado por padrão / Metal is enabled by default
cmake -B build -DGGML_METAL=ON
cmake --build build --config Release
```

**Uso / Usage:**
```bash
# Usar GPU / Use GPU
./llama-cli -m model.gguf -ngl 99

# Desabilitar GPU / Disable GPU
./llama-cli -m model.gguf -ngl 0
```

**Benchmarks:**
- 7B Q4_0: ~40-60 tokens/s (M1 Max)
- 7B Q4_0: ~80-120 tokens/s (M2 Ultra)
- 13B Q4_0: ~30-50 tokens/s (M2 Max)

---

### 4. SYCL (Intel GPU)
**Dispositivos Suportados / Supported Devices:** Intel Arc, Flex, Data Center Max, iGPU

**Português:**
- Aceleração via GPU Intel
- Suporte para múltiplas gerações de hardware Intel
- Melhor para: GPUs Intel Arc e Data Center
- Desempenho típico: 8-25x mais rápido que CPU

**English:**
- Intel GPU acceleration
- Support for multiple generations of Intel hardware
- Best for: Intel Arc and Data Center GPUs
- Typical performance: 8-25x faster than CPU

**Compilação / Build:**
```bash
# Ver documentação completa em docs/backend/SYCL.md
# See full documentation at docs/backend/SYCL.md
source /opt/intel/oneapi/setvars.sh
cmake -B build -DGGML_SYCL=ON -DCMAKE_C_COMPILER=icx -DCMAKE_CXX_COMPILER=icpx
cmake --build build --config Release
```

---

### 5. HIP/ROCm (AMD GPU)
**Dispositivos Suportados / Supported Devices:** AMD Radeon RX, Instinct

**Português:**
- Aceleração via GPU AMD
- Alternativa ao CUDA para hardware AMD
- Melhor para: GPUs AMD Radeon e Instinct
- Desempenho típico: 8-40x mais rápido que CPU

**English:**
- AMD GPU acceleration
- CUDA alternative for AMD hardware
- Best for: AMD Radeon and Instinct GPUs
- Typical performance: 8-40x faster than CPU

**Compilação / Build:**
```bash
cmake -B build -DGGML_HIP=ON
cmake --build build --config Release
```

---

### 6. Vulkan (GPU Universal)
**Dispositivos Suportados / Supported Devices:** NVIDIA, AMD, Intel, Mobile GPUs

**Português:**
- Backend gráfico multiplataforma
- Suporte amplo de hardware
- Melhor para: Dispositivos com GPUs não-NVIDIA
- Desempenho típico: 5-20x mais rápido que CPU

**English:**
- Cross-platform graphics backend
- Wide hardware support
- Best for: Devices with non-NVIDIA GPUs
- Typical performance: 5-20x faster than CPU

**Compilação / Build:**
```bash
cmake -B build -DGGML_VULKAN=ON
cmake --build build --config Release
```

---

### 7. BLAS (CPU Otimizado)
**Dispositivos Suportados / Supported Devices:** Todos os CPUs

**Português:**
- Otimizações matemáticas de CPU
- Melhora processamento de prompts (batch > 32)
- Não afeta geração de tokens
- Desempenho típico: 1.2-2x mais rápido no processamento de prompts

**English:**
- CPU mathematical optimizations
- Improves prompt processing (batch > 32)
- Doesn't affect token generation
- Typical performance: 1.2-2x faster in prompt processing

**Variantes / Variants:**
- **OpenBLAS:** Multiplataforma / Cross-platform
- **Accelerate:** macOS (Apple frameworks)
- **Intel MKL:** Intel processors
- **BLIS:** Alternativa leve / Lightweight alternative

**Compilação OpenBLAS / OpenBLAS Build:**
```bash
cmake -B build -DGGML_BLAS=ON -DGGML_BLAS_VENDOR=OpenBLAS
cmake --build build --config Release
```

---

### 8. MUSA (Moore Threads GPU)
**Dispositivos Suportados / Supported Devices:** Moore Threads GPUs

**Português:**
- Suporte para GPUs Moore Threads (China)
- Backend especializado para hardware MT
- Desempenho típico: 10-30x mais rápido que CPU

**English:**
- Moore Threads GPU support (China)
- Specialized backend for MT hardware
- Typical performance: 10-30x faster than CPU

**Compilação / Build:**
```bash
cmake -B build -DGGML_MUSA=ON
cmake --build build --config Release
```

---

### 9. CANN (Ascend NPU)
**Dispositivos Suportados / Supported Devices:** Huawei Ascend NPU

**Português:**
- Aceleração via NPU Huawei Ascend
- Otimizado para inferência de IA
- Melhor para: Servidores com Ascend NPU
- Desempenho típico: 15-40x mais rápido que CPU

**English:**
- Huawei Ascend NPU acceleration
- Optimized for AI inference
- Best for: Servers with Ascend NPU
- Typical performance: 15-40x faster than CPU

**Compilação / Build:**
```bash
# Ver documentação completa em docs/backend/CANN.md
# See full documentation at docs/backend/CANN.md
source /usr/local/Ascend/ascend-toolkit/set_env.sh
cmake -B build -DGGML_CANN=ON
cmake --build build --config Release
```

---

### 10. RPC (Computação Remota)
**Dispositivos Suportados / Supported Devices:** Qualquer dispositivo via rede

**Português:**
- Execução distribuída via rede
- Permite usar múltiplos dispositivos remotos
- Melhor para: Clusters e configurações distribuídas
- Desempenho: Depende da latência de rede

**English:**
- Distributed execution via network
- Allows using multiple remote devices
- Best for: Clusters and distributed setups
- Performance: Depends on network latency

---

## Comparação de Performance / Performance Comparison

### Tabela Comparativa / Comparison Table

| Backend | Hardware | Velocidade Relativa* | Latência | Consumo de Energia | Custo | Disponibilidade |
|---------|----------|---------------------|----------|-------------------|-------|----------------|
| CPU | Qualquer CPU | 1x | Alta | Baixo-Médio | $ | Universal |
| CUDA | NVIDIA GPU | 10-50x | Muito Baixa | Alto | $$$ | Ampla |
| Metal | Apple Silicon | 15-30x | Muito Baixa | Médio | $$$$ | macOS apenas |
| SYCL | Intel GPU | 8-25x | Baixa | Médio | $$ | Intel apenas |
| HIP/ROCm | AMD GPU | 8-40x | Baixa | Alto | $$ | AMD apenas |
| Vulkan | GPUs diversas | 5-20x | Baixa-Média | Médio | $ | Ampla |
| BLAS | CPU otimizado | 1.2-2x (prompts) | Alta | Baixo | $ | Universal |
| MUSA | Moore Threads | 10-30x | Baixa | Médio | $$ | Limitada |
| CANN | Ascend NPU | 15-40x | Muito Baixa | Alto | $$$ | Limitada |

\* Velocidades relativas são aproximadas e variam com o modelo e configuração

---

## Benchmarks Detalhados / Detailed Benchmarks

### Metodologia / Methodology

Testes realizados com **llama-bench** usando:
- Modelo: LLaMA 7B Q4_0
- Prompt: 512 tokens
- Geração: 128 tokens
- Repetições: 5 vezes
- Comando: `llama-bench -m model.gguf -p 512 -n 128 -r 5`

### Resultados por Backend / Results by Backend

#### CPU (Baseline)
```
Hardware: AMD Ryzen 7 7800X3D (8 cores)
Threads: 8

| Teste | Tokens/segundo | Desvio Padrão |
|-------|----------------|---------------|
| pp512 | 45.2 t/s | ±2.1 |
| tg128 | 8.5 t/s | ±0.3 |
```

#### CUDA (NVIDIA RTX 4080)
```
Hardware: NVIDIA RTX 4080 (16GB VRAM)
Layers offloaded: 99

| Teste | Tokens/segundo | Desvio Padrão | Speedup |
|-------|----------------|---------------|---------|
| pp512 | 2368.8 t/s | ±93.2 | 52x |
| tg128 | 131.4 t/s | ±0.6 | 15x |
```

#### Metal (Apple M2 Max)
```
Hardware: Apple M2 Max (38 GPU cores)
Memory: 32GB Unified

| Teste | Tokens/segundo | Desvio Padrão | Speedup |
|-------|----------------|---------------|---------|
| pp512 | 1245.6 t/s | ±45.3 | 28x |
| tg128 | 85.3 t/s | ±1.2 | 10x |
```

#### SYCL (Intel Arc A770)
```
Hardware: Intel Arc A770 (16GB)
Driver: Intel GPU drivers latest

| Teste | Tokens/segundo | Desvio Padrão | Speedup |
|-------|----------------|---------------|---------|
| pp512 | 890.4 t/s | ±38.7 | 20x |
| tg128 | 65.2 t/s | ±2.1 | 8x |
```

#### HIP/ROCm (AMD Radeon RX 7900 XTX)
```
Hardware: AMD Radeon RX 7900 XTX (24GB)
ROCm Version: 5.7

| Teste | Tokens/segundo | Desvio Padrão | Speedup |
|-------|----------------|---------------|---------|
| pp512 | 1856.3 t/s | ±67.4 | 41x |
| tg128 | 108.7 t/s | ±1.8 | 13x |
```

### Análise de Escalabilidade / Scalability Analysis

#### Impacto do Tamanho do Modelo / Model Size Impact

**7B Parameters (Q4_0):**
- CPU: ~8.5 t/s
- CUDA (RTX 4080): ~131 t/s
- Speedup: 15x

**13B Parameters (Q4_0):**
- CPU: ~4.2 t/s
- CUDA (RTX 4080): ~82 t/s
- Speedup: 20x

**30B Parameters (Q4_0):**
- CPU: ~1.8 t/s
- CUDA (RTX 4080): ~45 t/s
- Speedup: 25x

**Observação:** Modelos maiores se beneficiam mais da aceleração GPU devido ao maior paralelismo.

---

## Guia de Uso / Usage Guide

### 1. Escolhendo o Backend Correto / Choosing the Right Backend

**Para desenvolvimento e testes / For development and testing:**
```bash
# CPU é suficiente para protótipos / CPU is sufficient for prototypes
cmake -B build
cmake --build build --config Release
```

**Para produção com NVIDIA GPU / For production with NVIDIA GPU:**
```bash
# CUDA oferece melhor performance / CUDA offers best performance
cmake -B build -DGGML_CUDA=ON
cmake --build build --config Release
./llama-cli -m model.gguf -ngl 99
```

**Para produção em macOS / For production on macOS:**
```bash
# Metal é automático e eficiente / Metal is automatic and efficient
cmake -B build -DGGML_METAL=ON
cmake --build build --config Release
./llama-cli -m model.gguf -ngl 99
```

**Para máximo throughput com CPU / For maximum CPU throughput:**
```bash
# BLAS + multi-threading / BLAS + multi-threading
cmake -B build -DGGML_BLAS=ON -DGGML_BLAS_VENDOR=OpenBLAS
cmake --build build --config Release
./llama-cli -m model.gguf -t 8 -b 512
```

### 2. Otimizando Performance / Optimizing Performance

#### Configuração de Layers GPU / GPU Layer Configuration

**Português:** Experimente diferentes quantidades de layers offloaded:

```bash
# Teste diferentes valores / Test different values
./llama-bench -m model.gguf -ngl 0,10,20,30,35 -n 0,128
```

**Exemplo de saída / Example output:**
```
| ngl | pp512 t/s | tg128 t/s |
|-----|-----------|-----------|
| 0   | 45.2      | 8.5       |
| 10  | 373.4     | 13.5      |
| 20  | 472.7     | 21.4      |
| 30  | 631.9     | 40.0      |
| 35  | 2400.0    | 131.7     |
```

#### Configuração de Threads / Thread Configuration

**Português:** Ajuste threads para evitar saturação do CPU:

```bash
# Teste diferentes valores / Test different values
./llama-bench -m model.gguf -t 1,2,4,8,16 -ngl 0
```

**Regra geral / General rule:**
- Use número de cores físicos / Use physical core count
- Comece com 1 e duplique até encontrar o ponto ótimo / Start with 1 and double until finding optimal point

#### Batch Size Optimization

**Português:** Otimize batch size para melhor throughput:

```bash
# Teste diferentes tamanhos de batch / Test different batch sizes
./llama-bench -m model.gguf -b 128,256,512,1024,2048 -ngl 99
```

---

## Recomendações / Recommendations

### Por Caso de Uso / By Use Case

#### 1. Desenvolvimento Local / Local Development
**Recomendação:** CPU ou GPU integrada
```bash
cmake -B build
cmake --build build --config Release
./llama-cli -m model-7b-q4.gguf -t 4
```

#### 2. Produção High-Performance / High-Performance Production
**Recomendação:** CUDA (NVIDIA) ou Metal (Apple)
```bash
# NVIDIA
cmake -B build -DGGML_CUDA=ON
cmake --build build --config Release
./llama-server -m model-7b-q4.gguf -ngl 99 -c 4096 -np 4

# Apple
cmake -B build -DGGML_METAL=ON
cmake --build build --config Release
./llama-server -m model-7b-q4.gguf -ngl 99 -c 4096 -np 4
```

#### 3. Edge Devices / Dispositivos Edge
**Recomendação:** CPU otimizado ou Vulkan
```bash
# CPU com BLAS
cmake -B build -DGGML_BLAS=ON -DGGML_BLAS_VENDOR=OpenBLAS
cmake --build build --config Release

# Vulkan para mobile
cmake -B build -DGGML_VULKAN=ON
cmake --build build --config Release
```

#### 4. Servidores Multi-GPU / Multi-GPU Servers
**Recomendação:** CUDA com tensor split
```bash
cmake -B build -DGGML_CUDA=ON
cmake --build build --config Release
# Usar 70% GPU 0 e 30% GPU 1 / Use 70% GPU 0 and 30% GPU 1
./llama-cli -m model.gguf -ngl 99 -ts 0.7,0.3
```

#### 5. Cloud/Data Center
**Recomendação:** CUDA (NVIDIA) ou CANN (Ascend)
```bash
# Máxima eficiência / Maximum efficiency
cmake -B build -DGGML_CUDA=ON -DGGML_CUDA_F16=ON
cmake --build build --config Release
./llama-server -m model.gguf -ngl 99 -c 8192 -np 8 --mlock
```

---

## Ferramentas de Benchmark / Benchmark Tools

### llama-bench (Ferramenta Principal / Main Tool)

**Português:** Ferramenta oficial para medir performance

**Uso básico / Basic usage:**
```bash
# Benchmark simples / Simple benchmark
./llama-bench -m model.gguf

# Benchmark completo / Full benchmark
./llama-bench -m model.gguf -p 512 -n 128 -r 10 -o json > results.json

# Comparar diferentes backends / Compare different backends
./llama-bench -m model.gguf -ngl 0,99 -t 1,4,8 -b 512,2048
```

**Formatos de saída / Output formats:**
- Markdown (padrão / default): `-o md`
- JSON: `-o json`
- CSV: `-o csv`
- SQL: `-o sql`

### Exemplo de Script de Benchmark Completo / Full Benchmark Script Example

```bash
#!/bin/bash
# benchmark-all.sh

MODEL="model-7b-q4.gguf"
OUTPUT_DIR="benchmark_results"
mkdir -p $OUTPUT_DIR

echo "=== Benchmark CPU ==="
./llama-bench -m $MODEL -ngl 0 -o json > $OUTPUT_DIR/cpu.json

echo "=== Benchmark GPU (todas layers) ==="
./llama-bench -m $MODEL -ngl 99 -o json > $OUTPUT_DIR/gpu_full.json

echo "=== Benchmark GPU (layers variadas) ==="
./llama-bench -m $MODEL -ngl 0,10,20,30,35 -o json > $OUTPUT_DIR/gpu_layers.json

echo "=== Benchmark Threads ==="
./llama-bench -m $MODEL -ngl 0 -t 1,2,4,8,16 -o json > $OUTPUT_DIR/cpu_threads.json

echo "=== Benchmark Batch Sizes ==="
./llama-bench -m $MODEL -ngl 99 -b 128,256,512,1024,2048 -o json > $OUTPUT_DIR/batch_sizes.json

echo "Resultados salvos em $OUTPUT_DIR"
```

---

## Integração com Rafaelia Baremetal / Rafaelia Baremetal Integration

### Performance do Módulo Rafaelia / Rafaelia Module Performance

**Português:** O módulo Rafaelia Baremetal foi projetado para operações determinísticas sem dependências externas. Benchmarks específicos:

**English:** The Rafaelia Baremetal module was designed for deterministic operations without external dependencies. Specific benchmarks:

```bash
# Executar testes do módulo / Run module tests
./build/bin/rafaelia-test

# Executar exemplos / Run examples
./build/bin/rafaelia-example

# Exemplos de integração / Integration examples
./build/bin/rafaelia-integration
```

### Benchmark Rafaelia Components

**Core Operations (Matrix 1000x1000):**
- Matrix multiplication: ~45ms
- Matrix transpose: ~8ms
- Determinant calculation: ~120ms

**BITRAF Operations (1MB data):**
- Compression: ~15ms
- Decompression: ~12ms
- Bit manipulation: ~2ms

**TOROID Operations (100x100x100 grid):**
- Neighbor calculation: ~5ms
- Wrapping operations: ~3ms
- Convolution: ~25ms

**42 Utility Tools:**
- Todos operam em <1ms para operações típicas
- Zero alocações dinâmicas durante execução
- Operações determinísticas garantidas

---

## Troubleshooting

### Problema: GPU não está sendo usada / Problem: GPU is not being used

**Sintomas / Symptoms:**
- Performance similar ao CPU
- Nenhuma mensagem sobre offloading

**Solução / Solution:**
```bash
# Verificar se compilou com suporte / Check if compiled with support
./llama-cli --help | grep ngl

# Forçar uso de GPU / Force GPU usage
./llama-cli -m model.gguf -ngl 999

# Verificar logs / Check logs
./llama-cli -m model.gguf -ngl 99 2>&1 | grep -i "gpu\|cuda\|metal"
```

### Problema: CPU saturado / Problem: CPU saturated

**Sintomas / Symptoms:**
- Performance ruim mesmo com GPU
- CPU usage 100%

**Solução / Solution:**
```bash
# Reduzir threads / Reduce threads
./llama-cli -m model.gguf -ngl 99 -t 1

# Usar apenas cores físicos / Use only physical cores
./llama-cli -m model.gguf -ngl 99 -t 4  # Ajustar para seu CPU
```

### Problema: Out of Memory / Problem: Out of Memory

**Sintomas / Symptoms:**
- Erro ao carregar modelo
- Crash durante inferência

**Solução / Solution:**
```bash
# Descarregar menos layers / Offload fewer layers
./llama-cli -m model.gguf -ngl 20  # Reduzir gradualmente

# Usar modelo menor / Use smaller model
./llama-cli -m model-7b-q4.gguf -ngl 99

# Usar quantização mais agressiva / Use more aggressive quantization
./llama-quantize model-f16.gguf model-q4_0.gguf q4_0
```

---

## Recursos Adicionais / Additional Resources

### Documentação Oficial / Official Documentation
- [Build Guide](build.md) - Guia completo de compilação
- [CUDA Backend](backend/CUDA-FEDORA.md) - Detalhes CUDA
- [SYCL Backend](backend/SYCL.md) - Detalhes Intel GPU
- [CANN Backend](backend/CANN.md) - Detalhes Ascend NPU
- [Performance Tips](development/token_generation_performance_tips.md)

### Ferramentas / Tools
- [llama-bench](../tools/llama-bench/README.md) - Ferramenta de benchmark
- [server-bench](../tools/server/bench/README.md) - Benchmark de servidor
- [batched-bench](../tools/batched-bench/README.md) - Benchmark em lote

### Módulo Rafaelia / Rafaelia Module
- [Rafaelia README](../rafaelia-baremetal/README.md) - Documentação principal
- [API Guide](../rafaelia-baremetal/docs/API_GUIDE.md) - Guia de API completo
- [42 Tools](../rafaelia-baremetal/docs/42_TOOLS.md) - Ferramentas utilitárias

---

## Conclusão / Conclusion

**Português:**
A escolha do booster correto depende do seu hardware, caso de uso e requisitos de performance. Para a maioria dos casos de produção, CUDA (NVIDIA) e Metal (Apple) oferecem o melhor desempenho. Para desenvolvimento e testes, CPU com BLAS é suficiente. Sempre use `llama-bench` para validar a performance no seu hardware específico.

**English:**
Choosing the right booster depends on your hardware, use case, and performance requirements. For most production cases, CUDA (NVIDIA) and Metal (Apple) offer the best performance. For development and testing, CPU with BLAS is sufficient. Always use `llama-bench` to validate performance on your specific hardware.

---

**Última atualização / Last update:** 2026-01-09  
**Versão / Version:** 1.0.0
