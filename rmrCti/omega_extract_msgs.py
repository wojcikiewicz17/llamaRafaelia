#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import json, sys, argparse

def take_cap(dst_parts, s, cap, cur_len):
    if not s or cap <= 0:
        return cur_len
    need = cap - cur_len
    if need <= 0:
        return cur_len
    if len(s) > need:
        dst_parts.append(s[:need])
        return cap
    dst_parts.append(s)
    return cur_len + len(s)

def extract_text_capped(msg: dict, cap: int) -> str:
    if cap <= 0:
        return ""
    c = msg.get("content")
    if not c:
        return ""

    out = []
    cur = 0

    # dict: {parts:[...]} or {text:"..."}
    if isinstance(c, dict):
        parts = c.get("parts")
        if isinstance(parts, list):
            for p in parts:
                if cur >= cap: break
                if isinstance(p, str):
                    cur = take_cap(out, p, cap, cur)
                elif isinstance(p, dict):
                    t = p.get("text") or p.get("content") or p.get("value")
                    if isinstance(t, str):
                        cur = take_cap(out, t, cap, cur)
        else:
            t = c.get("text")
            if isinstance(t, str):
                cur = take_cap(out, t, cap, cur)

    # list: ["...", {...}]
    elif isinstance(c, list):
        for p in c:
            if cur >= cap: break
            if isinstance(p, str):
                cur = take_cap(out, p, cap, cur)
            elif isinstance(p, dict):
                t = p.get("text") or p.get("content") or p.get("value")
                if isinstance(t, str):
                    cur = take_cap(out, t, cap, cur)

    # str
    elif isinstance(c, str):
        cur = take_cap(out, c, cap, cur)

    return "".join(out).strip()

def has_tool_flag(msg: dict, role: str) -> int:
    if role == "tool":
        return 1
    if msg.get("tool_calls"):
        return 1
    md = msg.get("metadata") if isinstance(msg.get("metadata"), dict) else {}
    if md.get("tool") or md.get("tool_name") or md.get("tool_call_id"):
        return 1
    return 0

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--in_objs", default="omega_objs.jsonl")
    ap.add_argument("--out", default="omega_msgs.jsonl")
    ap.add_argument("--text_cap", type=int, default=1024)
    ap.add_argument("--progress", action="store_true")
    args = ap.parse_args()

    cap = max(0, args.text_cap)
    inp = args.in_objs
    out = args.out

    n_msgs = 0
    n_tool = 0
    roles = {}

    with open(inp, "r", encoding="utf-8", buffering=1024*1024) as f, \
         open(out, "w", encoding="utf-8", buffering=1024*1024) as fo:

        conv_i = -1
        for line in f:
            line = line.strip()
            if not line:
                continue
            conv_i += 1
            try:
                conv = json.loads(line)
            except Exception:
                continue

            title = conv.get("title","") if isinstance(conv, dict) else ""
            mapping = conv.get("mapping") if isinstance(conv, dict) else None
            if not isinstance(mapping, dict):
                continue

            for node_id, node in mapping.items():
                if not isinstance(node, dict):
                    continue
                msg = node.get("message")
                if not isinstance(msg, dict):
                    continue

                author = msg.get("author") if isinstance(msg.get("author"), dict) else {}
                role = author.get("role","") if isinstance(author, dict) else ""
                author_name = author.get("name","") if isinstance(author, dict) else ""

                text_cap = extract_text_capped(msg, cap)
                # text_len real (sem montar texto inteiro) -> fica como hint = len(text_cap) + (cap==len(text_cap)?"+":"")
                text_len_hint = len(text_cap)

                model = msg.get("model_slug","")
                md = msg.get("metadata") if isinstance(msg.get("metadata"), dict) else {}
                if not model:
                    model = md.get("model_slug","")
                dmodel = msg.get("default_model_slug","") or md.get("default_model_slug","")

                ht = has_tool_flag(msg, role)

                r = {
                    "conv_i": conv_i,
                    "conv_title": title,
                    "msg_id": msg.get("id", node_id),
                    "parent": node.get("parent", None),
                    "role": role,
                    "author_name": author_name,
                    "create_time": msg.get("create_time", None),
                    "model": model,
                    "dmodel": dmodel,
                    "text_len": text_len_hint,
                    "text": text_cap,
                    "has_tool": ht,
                }
                fo.write(json.dumps(r, ensure_ascii=False))
                fo.write("\n")

                n_msgs += 1
                roles[role] = roles.get(role, 0) + 1
                n_tool += 1 if ht else 0

                if args.progress and (n_msgs % 20000 == 0):
                    pct = (100.0 * n_tool / max(1, n_msgs))
                    print(f"[omega_extract_msgs] msgs={n_msgs} tool%={pct:.2f}", file=sys.stderr)

    pct = (100.0 * n_tool / max(1, n_msgs))
    print(f"[omega_extract_msgs] OK :: out={out}", file=sys.stderr)
    print(f"[omega_extract_msgs] msgs={n_msgs} tool_msgs={n_tool} pct={pct:.3f}", file=sys.stderr)
    print(f"[omega_extract_msgs] roles={roles}", file=sys.stderr)
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
