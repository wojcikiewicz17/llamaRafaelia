#!/usr/bin/env python3

import argparse
import math
import sys

from pathlib import Path

sys.path.append(str(Path(__file__).resolve().parents[1]))

from utils.lowlevel_tensor import (
    cosine_similarity,
    cosine_similarity_matrix,
    l2_norm,
    load_f32,
    mean,
    reshape_flat,
)

def load_embeddings_from_file(filename, n_embd):
    tensor = load_f32(filename)
    if len(tensor.data) == n_embd:
        return [tensor.data], 1, True
    if len(tensor.data) % n_embd != 0:
        raise ValueError("Embeddings file length is not divisible by embedding size.")
    n_tokens = len(tensor.data) // n_embd
    return reshape_flat(tensor.data, n_tokens, n_embd), n_tokens, False

def test_single_prompt_similarity(python_emb, cpp_emb, tokens, prompt):
    print("pytorch embeddings:");
    print(python_emb)
    print("llama.cpp embeddings:");
    print(cpp_emb)
    print(f"\n=== Prompt: '{prompt}' ===")
    print(f"Tokens: {tokens}")
    python_shape = (len(python_emb), len(python_emb[0]) if python_emb else 0)
    cpp_shape = (len(cpp_emb), len(cpp_emb[0]) if cpp_emb else 0)
    print(f"Embeddings shape: Python {python_shape}, llama.cpp {cpp_shape}")

    n_tokens = len(tokens)
    is_pooled = len(python_emb) == 1

    if is_pooled:
        print(f"\n[Pooled Embeddings Mode - comparing single sentence embeddings]")

        # 1. Direct embedding comparison for pooled embeddings
        print(f"\n1. Raw Embedding Magnitude Comparison:")
        py_mag = l2_norm(python_emb[0])
        cpp_mag = l2_norm(cpp_emb[0])
        ratio = py_mag / cpp_mag if cpp_mag > 0 else float('inf')
        print(f"   Pooled embedding: Python={py_mag:.3f}, llama.cpp={cpp_mag:.3f}, ratio={ratio:.3f}")

        # 2. Cross-model similarity for pooled embeddings
        print(f"\n2. Cross-Model Pooled Embedding Similarity:")
        sim = cosine_similarity(python_emb[0], cpp_emb[0])
        print(f"   Cosine similarity: {sim:.6f}")

        return {
            'cross_model_similarities': [sim],
            'similarity_matrix_diff': [[0.0]],
            'max_diff': 0.0,
            'mean_diff': 0.0,
            'rms_diff': 0.0
        }
    else:
        # Original per-token comparison logic
        # 1. Direct embedding comparison
        print(f"\n1. Raw Embedding Magnitude Comparison:")
        # Check if the distance of each token embedding from the origin and compare
        # if the vectors are on the same "sphere". This does not tell us about
        # direction (meaning of the token embedding), just magnitude.
        for i in range(n_tokens):
            py_mag = l2_norm(python_emb[i]) # calculate standard euclidean norm for Python embeddings
            cpp_mag = l2_norm(cpp_emb[i])   # calculate standard euclidean norm for llama.cpp embeddings
            ratio = py_mag / cpp_mag if cpp_mag > 0 else float('inf')
            print(f"   Token {i} ({tokens[i]}): Python={py_mag:.3f}, llama.cpp={cpp_mag:.3f}, ratio={ratio:.3f}")

        # 2. Cosine similarity between tokens within each model
        # Here we check the direction of token embeddings to see if the have the
        # same meaning (similarity). This is done by calculating cosine similarity
        # of a pair of token embeddings within each model.
        print(f"\n2. Within-Model Token Similarities:")
        print("   Python model:")
        for i in range(n_tokens):
            for j in range(i+1, n_tokens):
                sim = cosine_similarity(python_emb[i], python_emb[j])
                print(f"     {tokens[i]} ↔ {tokens[j]}: {sim:.4f}")

        print("   llama.cpp model:")
        for i in range(n_tokens):
            for j in range(i+1, n_tokens):
                sim = cosine_similarity(cpp_emb[i], cpp_emb[j])
                print(f"     {tokens[i]} ↔ {tokens[j]}: {sim:.4f}")

        # 3. Cross-model similarity (same token position)
        print(f"\n3. Cross-Model Same-Token Similarities:")
        for i in range(n_tokens):
            sim = cosine_similarity(python_emb[i], cpp_emb[i])
            print(f"   Token {i} ({tokens[i]}): {sim:.4f}")

        # 4. Similarity matrix comparison
        print(f"\n4. Similarity Matrix Differences:")
        py_sim_matrix = cosine_similarity_matrix(python_emb)
        cpp_sim_matrix = cosine_similarity_matrix(cpp_emb)
        diff_matrix = [
            [abs(py_sim_matrix[i][j] - cpp_sim_matrix[i][j]) for j in range(n_tokens)]
            for i in range(n_tokens)
        ]
        max_diff = max(max(row) for row in diff_matrix)
        mean_diff = mean([value for row in diff_matrix for value in row])
        rms_diff = math.sqrt(mean([value * value for row in diff_matrix for value in row]))

        print(f"   Max difference: {max_diff:.4f}")
        print(f"   Mean difference: {mean_diff:.4f}")
        print(f"   RMS difference: {rms_diff:.4f}")

        return {
            'cross_model_similarities': [cosine_similarity(python_emb[i], cpp_emb[i]) for i in range(n_tokens)],
            'similarity_matrix_diff': diff_matrix,
            'max_diff': max_diff,
            'mean_diff': mean_diff,
            'rms_diff': rms_diff
        }

def read_prompt_from_file(file_path):
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            return f.read().strip()
    except FileNotFoundError:
        print(f"Error: Prompts file '{file_path}' not found")
        exit(1)
    except Exception as e:
        print(f"Error reading prompts file: {e}")
        exit(1)

def read_tokens_from_file(file_path):
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            return [line.strip() for line in f if line.strip()]
    except FileNotFoundError:
        print(f"Error: Tokens file '{file_path}' not found")
        exit(1)
    except Exception as e:
        print(f"Error reading tokens file: {e}")
        exit(1)

def main():
    parser = argparse.ArgumentParser(description='Test semantic similarity between Python and llama.cpp embeddings')
    parser.add_argument('--python-embeddings', '-pe', help='Path to pytorch embeddings "logits" binary file')
    parser.add_argument('--cpp-embeddings', '-ce', help='Path to llama.cpp embeddings "logits" binary file')
    parser.add_argument('--prompt', '-p', default='Hello world today', help='Test prompt')
    parser.add_argument('--prompts-file', '-pf', help='Path to file containing prompts')
    parser.add_argument('--tokens', '-t', help='Comma-separated list of tokens used in the prompt')
    parser.add_argument('--tokens-file', '-tf', help='Path to file containing tokens (one per line)')
    parser.add_argument('--n-embd', '-ne', type=int, required=True, help='Embedding dimension')

    args = parser.parse_args()

    if args.prompts_file:
        prompt = read_prompt_from_file(args.prompts_file)
    else:
        prompt = args.prompt

    print("Semantic Similarity Test Between Python and llama.cpp Embedding Models")
    print("=" * 70)

    # Single prompt detailed comparison
    print(f"\nTesting with prompt: '{prompt}'")

    # Load binary embeddings from data directory.
    python_embeddings, n_tokens, is_pooled = load_embeddings_from_file(args.python_embeddings, args.n_embd)
    cpp_embeddings, n_tokens_cpp, is_pooled_cpp = load_embeddings_from_file(args.cpp_embeddings, args.n_embd)
    if n_tokens != n_tokens_cpp:
        raise ValueError("Embedding files have mismatched token counts.")
    if is_pooled != is_pooled_cpp:
        raise ValueError("Embedding files disagree on pooled vs per-token format.")

    if args.tokens:
        tokens = [token.strip() for token in args.tokens.split(",") if token.strip()]
    elif args.tokens_file:
        tokens = read_tokens_from_file(args.tokens_file)
    else:
        tokens = [f"tok_{i}" for i in range(n_tokens)]
    if len(tokens) != n_tokens:
        raise ValueError("Token count does not match embeddings length.")
    print(f"n_tokens: {n_tokens}")
    print(f"hidden_size: {args.n_embd}")

    # Run comparison
    results = test_single_prompt_similarity(python_embeddings, cpp_embeddings, tokens, prompt)

    # Summary
    print(f"\n=== SUMMARY ===")
    avg_cross_sim = mean(results['cross_model_similarities'])
    print(f"Average cross-model similarity: {avg_cross_sim:.4f}")
    print(f"Similarity matrix RMS difference: {results['rms_diff']:.4f}")

    # Quality assessment
    if avg_cross_sim > 0.95:
        print("✅ EXCELLENT: Models are highly similar")
    elif avg_cross_sim > 0.90:
        print("✅ VERY GOOD: Models are very similar")
    elif avg_cross_sim > 0.80:
        print("⚠️  GOOD: Models are reasonably similar")
    elif avg_cross_sim > 0.70:
        print("⚠️  FAIR: Models have some differences")
    else:
        print("❌ POOR: Models are significantly different")

if __name__ == "__main__":
    main()
