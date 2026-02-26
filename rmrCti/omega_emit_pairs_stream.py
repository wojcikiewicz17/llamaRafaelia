#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
OMEGA EMIT (streaming) — export OpenAI conversations.json
Lê JSON gigante via STDIN e extrai:
- create_time (ts)
- texto em content.parts[] (strings)
Gera TSV: token \t ts \t msg_id \t count

Sem json.load. Sem dependências. Termux-friendly.
"""

import sys, re

TOKEN_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]{2,}")

def read_json_string(b, i):
    # lê string JSON a partir de aspas (b[i] == ord('"'))
    i += 1
    out = []
    n = len(b)
    while i < n:
        c = b[i]
        if c == 34:  # "
            return "".join(out), i + 1
        if c == 92:  # backslash
            i += 1
            if i >= n: break
            esc = b[i]
            if esc == 34: out.append('"')
            elif esc == 92: out.append("\\")
            elif esc == 47: out.append("/")
            elif esc == 98: out.append("\b")
            elif esc == 102: out.append("\f")
            elif esc == 110: out.append("\n")
            elif esc == 114: out.append("\r")
            elif esc == 116: out.append("\t")
            elif esc == 117 and i + 4 < n:  # \uXXXX
                hex4 = b[i+1:i+5].decode("ascii", "ignore")
                try:
                    out.append(chr(int(hex4, 16)))
                except Exception:
                    pass
                i += 4
            else:
                # escape desconhecido: ignora
                pass
        else:
            # UTF-8 bruto (já vem como bytes)
            out.append(chr(c))
        i += 1
    return "".join(out), i

def skip_ws(b, i):
    n = len(b)
    while i < n and b[i] in (9,10,13,32):
        i += 1
    return i

def read_number(b, i):
    n = len(b)
    j = i
    while j < n and (48 <= b[j] <= 57 or b[j] in (45,46)): # - . digits
        j += 1
    return b[i:j].decode("ascii", "ignore"), j

def tokenize(text):
    d = {}
    for t in TOKEN_RE.findall(text):
        tl = t.lower()
        if len(tl) < 3 or len(tl) > 64:
            continue
        d[tl] = d.get(tl, 0) + 1
    return d

def main():
    out_path = sys.argv[1] if len(sys.argv) > 1 else "pairs.tsv"
    # Lê em chunks, mas precisa varrer padrões; fazemos buffer deslizante
    CHUNK = 1 << 20  # 1MB
    buf = b""
    msg_id = 0
    ts_last = ""

    with open(out_path, "w", encoding="utf-8") as out:
        while True:
            chunk = sys.stdin.buffer.read(CHUNK)
            if not chunk:
                break
            buf += chunk
            # mantém buffer sob controle
            if len(buf) > 8 * CHUNK:
                buf = buf[-4*CHUNK:]

            i = 0
            n = len(buf)

            # varredura simples por chaves relevantes
            while i < n:
                # procura "create_time"
                if buf[i:i+13] == b'"create_time"':
                    i += 13
                    i = skip_ws(buf, i)
                    if i < n and buf[i] == 58: # :
                        i += 1
                        i = skip_ws(buf, i)
                        num, j = read_number(buf, i)
                        if num:
                            ts_last = num
                        i = j
                    continue

                # procura "parts"
                if buf[i:i+7] == b'"parts"':
                    i += 7
                    i = skip_ws(buf, i)
                    if i < n and buf[i] == 58:
                        i += 1
                        i = skip_ws(buf, i)
                        if i < n and buf[i] == 91: # [
                            i += 1
                            texts = []
                            # lê strings até fechar ]
                            while i < n:
                                i = skip_ws(buf, i)
                                if i < n and buf[i] == 93: # ]
                                    i += 1
                                    break
                                if i < n and buf[i] == 34: # "
                                    s, i = read_json_string(buf, i)
                                    if s.strip():
                                        texts.append(s)
                                    i = skip_ws(buf, i)
                                    if i < n and buf[i] == 44: # ,
                                        i += 1
                                        continue
                                else:
                                    # pula qualquer coisa estranha
                                    i += 1

                            if texts:
                                msg_id += 1
                                text = "\n".join(texts)
                                toks = tokenize(text)
                                ts = ts_last if ts_last else "0"
                                for t, c in toks.items():
                                    out.write(f"{t}\t{ts}\t{msg_id}\t{c}\n")
                    continue

                i += 1

    print("OK :: wrote", out_path, "msgs~", msg_id, file=sys.stderr)

if __name__ == "__main__":
    main()
