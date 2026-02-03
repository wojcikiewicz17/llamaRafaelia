# SPEC_GEOMETRY_NUCLEUS_V1

## Objetivo

Definir um **núcleo modular** de matemática e geometria para suportar **inferências** e **blendings** (combinações geométricas), traduzíveis por **indexação** e **queries** em espaços geométricos. Este documento parte do diagrama fornecido (sólidos, anéis e órbitas) como metáfora de **camadas geométricas** e **fluxos de transformação**.

## Princípios

1. **Geometria como índice**: a forma é um endereço, e a métrica define a consulta.
2. **Módulos desacoplados**: matemática, geometria, inferência e blending têm interfaces claras.
3. **Composição por camadas**: módulos podem ser empilhados (stacks) sem colapsar a topologia.
4. **Invariância parcial**: operações preservam propriedades selecionadas (ex.: volume, orientação, simetria).
5. **Núcleo low-level**: o núcleo executa em C/ASM, sem dependências externas.
6. **Casca em Python**: Python é apenas uma camada de orquestração (shell), sem lógica crítica.
7. **Interoperabilidade preservada**: a lógica atual não deve ser quebrada; mudanças são incrementais.

## Restrições de Implementação

- **Sem bibliotecas externas** (inclusive ML frameworks como PyTorch).
- **Sem classes** (evitar padrões OO).
- **Nova metodologia**: pipeline orientado a dados, com estados explícitos e tabelas de operação.
- **Funções mínimas e determinísticas**: apenas o estritamente necessário para o núcleo; preferir
  blocos de operação descritos por tabelas, com execução sequencial e previsível.

## Núcleo Modular

### 1) Módulo de Fundamentos Matemáticos (MathCore)

Responsável por tipos e operações fundamentais.

**Tipos base**
- Escalar (ℝ), vetor (ℝⁿ), matriz (ℝⁿˣᵐ)
- Tensor denso e esparso

**Operações**
- Normas e métricas (L1, L2, L∞, geodésica)
- Projeções, bases ortonormais
- Decomposições (SVD, QR, autovalores)

### 2) Módulo de Geometria (GeoCore)

Define o espaço e os primitivos geométricos.

**Primitivos**
- Plano, esfera, cubo, cone, cilindro, toro
- Curvas paramétricas e superfícies

**Transformações**
- Translação, rotação, escala, cisalhamento
- Transformações não lineares (warp, twist)

**Topologias**
- Euclidiana, toroidal, hiperesférica
- Camadas orbitais (traços do diagrama)

### 3) Módulo de Inferência Geométrica (GeoInference)

Realiza deduções e relações entre formas.

**Inferências**
- Similaridade (shape matching)
- Contenção, interseção, proximidade
- Alinhamento e coerência orbital

**Saídas**
- Grafos de relações
- Pontuações de coerência geométrica

### 4) Módulo de Blending (GeoBlend)

Combina formas com controle de continuidade.

**Modos**
- Linear (LERP de vértices)
- Espectral (blend por frequência)
- Topológico (morfologia guiada)

**Restrições**
- Preservar volume ou área
- Preservar simetrias do núcleo

## Indexação e Queries Geométricas

O núcleo é traduzido para **índices geométricos** que permitem consulta rápida.

### Estruturas de índice

- **GeoHash 3D/ND**: endereçamento por célula espacial.
- **KD-tree / R-tree**: busca por vizinhança.
- **Orbit-index**: índices por camadas orbitais (anéis do diagrama).

### Consultas (queries)

**Exemplos**
- `find(similar, shape=cubo, tolerance=0.1)`
- `intersect(shape=torus, region=orbit:3)`
- `blend(shapeA=cone, shapeB=cyl, mode=topological)`

## Fluxo de Dados

```
Input (formas/param) 
   -> MathCore (métricas) 
   -> GeoCore (representação)
   -> GeoInference (relações)
   -> GeoBlend (combinações)
   -> Index/Query (busca e recuperação)
```

## Metodologia de Núcleo (Low-level)

Para atender ao requisito de **C/ASM low-level** e evitar dependências externas:

1. **Estado explícito**: todo módulo expõe um `state` em estruturas C simples.
2. **Tabelas de operação**: o núcleo executa sequências declaradas em tabelas (ex.: opcode + parâmetros).
3. **Sem OOP**: não usar classes ou herança; preferir composição por structs.
4. **Interoperabilidade**: a casca Python apenas invoca o núcleo e serializa parâmetros/resultados.
5. **ASM opcional**: rotinas críticas (ex.: métricas, blending) podem ter variantes em ASM.

## Tradução do Diagrama

O diagrama sugere:
- **Sólidos**: primitivos geométricos.
- **Anéis/órbitas**: camadas topológicas e índices orbitais.
- **Fluxos luminosos**: transformações e inferências em trânsito.

Esses elementos são mapeados para o núcleo modular por meio de:
1. **Primitivos** (GeoCore)
2. **Métricas** (MathCore)
3. **Relações** (GeoInference)
4. **Misturas** (GeoBlend)
5. **Busca** (Index/Query)

## Próximos Passos

- Definir API em C/ASM para cada módulo seguindo a metodologia de tabelas.
- Implementar protótipos de indexação sem dependências externas.
- Criar exemplos com consultas sobre formas básicas, com Python apenas como casca.
