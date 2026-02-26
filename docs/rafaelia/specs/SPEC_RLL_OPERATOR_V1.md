# RLL Operator v1 (SPEC)

## Objetivo
Definir o contrato operacional mínimo do template `models/templates/rafaelia-rll.jinja`: toda resposta do assistente deve vir em quatro blocos fixos para facilitar validação manual e automação.

## Contrato obrigatório de saída
Toda resposta deve conter **exatamente** estes blocos, nesta ordem:
1. `CONTEXT`
2. `STATE`
3. `TRANSITION`
4. `RESULT`

Regras:
- Não renomear títulos.
- Não remover blocos.
- Não inserir bloco adicional.
- O conteúdo pode variar, mas os quatro blocos devem sempre existir.

## Exemplo concreto (prompt renderizado)
Exemplo de entrada (mensagens):
- system: `Modo operador RLL. Responder curto.`
- user: `Qual o próximo passo para validar o formato?`

Prompt final renderizado pelo template:

```text
<s>System: Operador RLL obrigatório. Em TODA resposta, use exatamente os quatro blocos abaixo, nesta ordem e com estes títulos literais: CONTEXT, STATE, TRANSITION, RESULT. Não omita blocos, não renomeie blocos e não adicione blocos extras.
System: Modo operador RLL. Responder curto.
User: Qual o próximo passo para validar o formato?
Assistant:
```

## Resposta esperada (formato)
Exemplo válido mínimo:

```text
CONTEXT
Validação manual do contrato de saída RLL em execução.

STATE
Template ativo e instrução de quatro blocos presente no bloco de sistema.

TRANSITION
Executar uma pergunta simples no llama-cli/llama-server e verificar presença e ordem dos quatro títulos.

RESULT
Formato confirmado: CONTEXT → STATE → TRANSITION → RESULT.
```

## Checklist manual (`llama-cli` / `llama-server`)
- Renderizar prompt com `--chat-template-file models/templates/rafaelia-rll.jinja`.
- Enviar pergunta curta.
- Validar que a resposta contém os quatro títulos na ordem correta.
- Reprovar caso falte, troque nome, ou inclua bloco extra.
