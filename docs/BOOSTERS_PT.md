# Boosters de Performance - Guia Completo

## 📚 Visão Geral

Este documento detalha os **boosters de performance** (também conhecidos como **backends** ou **aceleradores**) disponíveis no llama.cpp. Boosters são tecnologias que utilizam hardware especializado para acelerar drasticamente a inferência de modelos LLM.

> **💡 Nota:** Para a versão bilíngue completa (Português/Inglês) com todos os detalhes técnicos, veja [BOOSTERS.md](BOOSTERS.md)

---

## 🚀 O que são Boosters?

Boosters são backends de hardware que multiplicam a velocidade de inferência de modelos de linguagem. Em vez de processar tudo na CPU tradicional, eles utilizam:

- **GPUs** (Placas de vídeo NVIDIA, AMD, Intel, Apple)
- **NPUs** (Unidades de Processamento Neural especializadas)
- **Aceleradores especializados** (Huawei Ascend, Moore Threads, etc.)
- **Otimizações matemáticas** (BLAS para CPUs)

---

## 📊 Comparação Rápida

| Backend | Hardware Necessário | Ganho de Performance | Custo | Disponibilidade |
|---------|-------------------|---------------------|-------|----------------|
| **CPU** | Qualquer processador | 1x (baseline) | $ | ✅ Universal |
| **CUDA** | GPU NVIDIA | 10-50x mais rápido | $$$ | ✅ Ampla |
| **Metal** | Apple Silicon (M1/M2/M3/M4) | 15-30x mais rápido | $$$$ | 🍎 macOS apenas |
| **SYCL** | GPU Intel Arc/Flex | 8-25x mais rápido | $$ | 🔷 Intel apenas |
| **HIP/ROCm** | GPU AMD Radeon | 8-40x mais rápido | $$ | 🔴 AMD apenas |
| **Vulkan** | Qualquer GPU | 5-20x mais rápido | $ | ✅ Ampla |
| **BLAS** | CPU otimizada | 1.2-2x (prompts) | $ | ✅ Universal |

---

## 🎯 Qual Booster Devo Usar?

### Para Desenvolvimento e Testes
```bash
# CPU é suficiente
cmake -B build
cmake --build build --config Release
./llama-cli -m modelo.gguf
```

### Se Você Tem GPU NVIDIA
```bash
# CUDA oferece a melhor performance
cmake -B build -DGGML_CUDA=ON
cmake --build build --config Release
./llama-cli -m modelo.gguf -ngl 99
```
**Ganho esperado:** 10-50x mais rápido

### Se Você Tem Mac com Apple Silicon (M1/M2/M3/M4)
```bash
# Metal é automático no macOS
cmake -B build -DGGML_METAL=ON
cmake --build build --config Release
./llama-cli -m modelo.gguf -ngl 99
```
**Ganho esperado:** 15-30x mais rápido

### Se Você Tem GPU AMD
```bash
# Use HIP/ROCm
cmake -B build -DGGML_HIP=ON
cmake --build build --config Release
./llama-cli -m modelo.gguf -ngl 99
```
**Ganho esperado:** 8-40x mais rápido

### Se Você Tem GPU Intel Arc
```bash
# Use SYCL
source /opt/intel/oneapi/setvars.sh
cmake -B build -DGGML_SYCL=ON -DCMAKE_C_COMPILER=icx -DCMAKE_CXX_COMPILER=icpx
cmake --build build --config Release
./llama-cli -m modelo.gguf -ngl 99
```
**Ganho esperado:** 8-25x mais rápido

### Para Máximo Throughput com CPU Apenas
```bash
# Use BLAS para otimização matemática
cmake -B build -DGGML_BLAS=ON -DGGML_BLAS_VENDOR=OpenBLAS
cmake --build build --config Release
./llama-cli -m modelo.gguf -t 8
```
**Ganho esperado:** 1.2-2x no processamento de prompts

---

## 📈 Benchmarks Reais

### Modelo LLaMA 7B Q4_0 - Geração de Tokens

| Hardware | Tokens por Segundo | Ganho |
|----------|-------------------|-------|
| CPU (AMD Ryzen 7800X3D) | 8.5 t/s | 1x |
| CUDA (RTX 4080) | 131 t/s | **15x** ⚡ |
| Metal (M2 Max) | 85 t/s | **10x** ⚡ |
| SYCL (Intel Arc A770) | 65 t/s | **8x** ⚡ |
| ROCm (Radeon RX 7900 XTX) | 109 t/s | **13x** ⚡ |

### Modelo LLaMA 13B Q4_0 - Geração de Tokens

| Hardware | Tokens por Segundo | Ganho |
|----------|-------------------|-------|
| CPU | 4.2 t/s | 1x |
| CUDA (RTX 4080) | 82 t/s | **20x** ⚡ |

### Modelo LLaMA 30B Q4_0 - Geração de Tokens

| Hardware | Tokens por Segundo | Ganho |
|----------|-------------------|-------|
| CPU | 1.8 t/s | 1x |
| CUDA (RTX 4080) | 45 t/s | **25x** ⚡ |

**Observação:** Modelos maiores se beneficiam AINDA MAIS da aceleração GPU! 🚀

---

## 🛠️ Como Otimizar Performance

### 1. Ajustar Layers Offloaded (-ngl)

O parâmetro `-ngl` controla quantas camadas do modelo são enviadas para a GPU:

```bash
# Enviar TODAS as camadas para GPU (recomendado)
./llama-cli -m modelo.gguf -ngl 99

# Enviar apenas 20 camadas (útil se VRAM for limitada)
./llama-cli -m modelo.gguf -ngl 20

# Não usar GPU (apenas CPU)
./llama-cli -m modelo.gguf -ngl 0
```

**Teste diferentes valores:**
```bash
# Descobrir o valor ideal para seu hardware
./llama-bench -m modelo.gguf -ngl 0,10,20,30,35
```

### 2. Ajustar Threads (-t)

Para CPUs, o número de threads é crítico:

```bash
# Usar 1 thread (mínimo)
./llama-cli -m modelo.gguf -t 1

# Usar número de cores físicos (recomendado)
./llama-cli -m modelo.gguf -t 8

# Teste para encontrar o ideal
./llama-bench -m modelo.gguf -t 1,2,4,8,16 -ngl 0
```

**Regra geral:**
- Comece com número de **cores físicos** (não lógicos)
- Se performance for ruim, comece com 1 e vá dobrando
- Threads demais = CPU saturado = performance PIOR

### 3. Ajustar Batch Size (-b)

Para processamento de prompts longos:

```bash
# Testar diferentes batch sizes
./llama-bench -m modelo.gguf -b 128,256,512,1024,2048 -ngl 99
```

---

## 🎓 Casos de Uso Recomendados

### 💻 Desenvolvimento Local
**Hardware:** CPU ou GPU integrada  
**Booster:** CPU padrão ou BLAS  
**Comando:**
```bash
cmake -B build
cmake --build build --config Release
./llama-cli -m modelo-7b-q4.gguf -t 4
```

### 🏢 Produção High-Performance
**Hardware:** GPU NVIDIA ou Apple Silicon  
**Booster:** CUDA ou Metal  
**Comando:**
```bash
# NVIDIA
cmake -B build -DGGML_CUDA=ON
cmake --build build --config Release
./llama-server -m modelo-7b-q4.gguf -ngl 99 -c 4096 -np 4

# Apple
cmake -B build -DGGML_METAL=ON
cmake --build build --config Release
./llama-server -m modelo-7b-q4.gguf -ngl 99 -c 4096 -np 4
```

### 📱 Dispositivos Edge/Mobile
**Hardware:** ARM CPUs ou GPUs mobile  
**Booster:** CPU otimizado ou Vulkan  
**Comando:**
```bash
# CPU com BLAS
cmake -B build -DGGML_BLAS=ON -DGGML_BLAS_VENDOR=OpenBLAS
cmake --build build --config Release

# Vulkan para mobile
cmake -B build -DGGML_VULKAN=ON
cmake --build build --config Release
```

### ☁️ Cloud/Data Center
**Hardware:** GPUs NVIDIA de servidor  
**Booster:** CUDA otimizado  
**Comando:**
```bash
cmake -B build -DGGML_CUDA=ON -DGGML_CUDA_F16=ON
cmake --build build --config Release
./llama-server -m modelo.gguf -ngl 99 -c 8192 -np 8 --mlock
```

### 🖥️ Múltiplas GPUs
**Hardware:** Servidor com 2+ GPUs  
**Booster:** CUDA com tensor split  
**Comando:**
```bash
# Usar 70% da GPU 0 e 30% da GPU 1
./llama-cli -m modelo.gguf -ngl 99 -ts 0.7,0.3
```

---

## 🔧 Ferramenta de Benchmark

### llama-bench - Ferramenta Oficial

**Teste básico:**
```bash
./llama-bench -m modelo.gguf
```

**Teste completo com todas as combinações:**
```bash
./llama-bench -m modelo.gguf \
  -p 512 \           # Prompt de 512 tokens
  -n 128 \           # Gerar 128 tokens
  -r 10 \            # 10 repetições
  -o json > resultados.json
```

**Comparar CPU vs GPU:**
```bash
./llama-bench -m modelo.gguf -ngl 0,99
```

**Testar diferentes configurações de threads:**
```bash
./llama-bench -m modelo.gguf -t 1,2,4,8,16 -ngl 0
```

**Testar layers offload gradualmente:**
```bash
./llama-bench -m modelo.gguf -ngl 0,10,20,30,35
```

---

## 🩺 Troubleshooting (Resolução de Problemas)

### Problema: GPU não está sendo usada

**Sintomas:** Performance igual ao CPU

**Solução:**
```bash
# Verificar se compilou com suporte a GPU
./llama-cli --help | grep ngl

# Forçar uso máximo da GPU
./llama-cli -m modelo.gguf -ngl 999

# Ver logs de offloading
./llama-cli -m modelo.gguf -ngl 99 2>&1 | grep -i "gpu\|cuda\|metal"
```

**Você deve ver algo como:**
```
llama_model_load_internal: [cublas] offloading 35 layers to GPU
llama_model_load_internal: [cublas] total VRAM used: 4521 MB
```

### Problema: CPU está saturado (100% uso)

**Sintomas:** Performance ruim mesmo com GPU

**Solução:**
```bash
# Reduzir threads para 1
./llama-cli -m modelo.gguf -ngl 99 -t 1

# Ou usar apenas cores físicos
./llama-cli -m modelo.gguf -ngl 99 -t 4
```

### Problema: Out of Memory (Sem memória)

**Sintomas:** Erro ao carregar modelo ou crash

**Solução:**
```bash
# Opção 1: Reduzir layers offloaded
./llama-cli -m modelo.gguf -ngl 20

# Opção 2: Usar modelo menor
./llama-cli -m modelo-7b-q4.gguf -ngl 99

# Opção 3: Usar quantização mais agressiva
./llama-quantize modelo-f16.gguf modelo-q4_0.gguf q4_0
```

### Problema: Performance pior que esperado

**Diagnóstico:**
```bash
# Teste sistemático
./llama-bench -m modelo.gguf -ngl 0,99 -t 1,4,8 -o json > diagnostico.json
```

**Checklist:**
- ✅ Compilou com o backend correto?
- ✅ Está usando `-ngl 99`?
- ✅ Threads configurado corretamente?
- ✅ GPU tem VRAM suficiente?
- ✅ Drivers da GPU atualizados?

---

## 📚 Recursos Adicionais

### Documentação Técnica Detalhada
- 📖 [BOOSTERS.md](BOOSTERS.md) - Versão completa bilíngue com todos os detalhes técnicos
- 🏗️ [build.md](build.md) - Guia completo de compilação
- ⚡ [Performance Tips](development/token_generation_performance_tips.md) - Dicas de otimização

### Documentação de Backends Específicos
- 🟢 [CUDA Backend](backend/CUDA-FEDORA.md)
- 🔷 [SYCL Backend](backend/SYCL.md) (Intel)
- 🟠 [CANN Backend](backend/CANN.md) (Ascend)
- 🟣 [BLIS Backend](backend/BLIS.md)

### Ferramentas de Benchmark
- [llama-bench](../tools/llama-bench/README.md) - Benchmark principal
- [server-bench](../tools/server/bench/README.md) - Benchmark do servidor
- [batched-bench](../tools/batched-bench/README.md) - Benchmark em lote

### Módulo Rafaelia
- [Rafaelia README](../rafaelia-baremetal/README.md) - Documentação do módulo baremetal
- [API Guide](../rafaelia-baremetal/docs/API_GUIDE.md) - Guia completo da API
- [42 Tools](../rafaelia-baremetal/docs/42_TOOLS.md) - Ferramentas utilitárias

---

## 🎯 Resumo Rápido

| Tenho... | Devo Usar... | Comando |
|----------|--------------|---------|
| GPU NVIDIA | CUDA | `cmake -B build -DGGML_CUDA=ON` |
| Mac M1/M2/M3/M4 | Metal | `cmake -B build -DGGML_METAL=ON` |
| GPU AMD | HIP/ROCm | `cmake -B build -DGGML_HIP=ON` |
| GPU Intel Arc | SYCL | Ver [SYCL.md](backend/SYCL.md) |
| Apenas CPU | BLAS | `cmake -B build -DGGML_BLAS=ON` |
| Qualquer GPU | Vulkan | `cmake -B build -DGGML_VULKAN=ON` |

**Depois de compilar, sempre use:**
```bash
./llama-cli -m modelo.gguf -ngl 99
```

O parâmetro `-ngl 99` envia o máximo possível de camadas para a GPU! 🚀

---

## 💡 Dica Final

**Sempre teste sua configuração com llama-bench:**
```bash
# Teste rápido
./llama-bench -m seu-modelo.gguf

# Teste completo (salva resultados)
./llama-bench -m seu-modelo.gguf -ngl 0,99 -t 1,4,8 -o json > meu-teste.json
```

Isso vai te mostrar exatamente qual configuração funciona melhor no SEU hardware! 📊

---

**Última atualização:** 2026-01-09  
**Versão:** 1.0.0

---

## 📞 Precisa de Mais Ajuda?

- 📖 Documentação completa: [BOOSTERS.md](BOOSTERS.md)
- 🐛 Issues: [GitHub Issues](https://github.com/rafaelmeloreisnovo/llamaRafaelia/issues)
- 💬 Discussões: [GitHub Discussions](https://github.com/ggml-org/llama.cpp/discussions)
