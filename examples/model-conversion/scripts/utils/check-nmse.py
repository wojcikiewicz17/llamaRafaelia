#!/usr/bin/env python3

import sys
import os
import argparse
import math
from pathlib import Path

sys.path.append(str(Path(__file__).resolve().parents[1]))

from utils.lowlevel_tensor import load_f32, mean_abs_diff, max_abs_diff, nmse

def calculate_nmse(reference, test):
    return nmse(reference, test)

def load_logits(file_path):
    return load_f32(file_path)

def interpret_nmse(nmse):
    """Provide interpretation of NMSE value"""
    if nmse == 0:
        return "Perfect match", "🎉"
    elif nmse < 1e-6:
        return "Essentially identical", "✅"
    elif nmse < 1e-4:
        return "Excellent match", "✅"
    elif nmse < 1e-3:
        return "Very good match", "👍"
    elif nmse < 1e-2:
        return "Good match", "👍"
    elif nmse < 0.1:
        return "Acceptable match", "⚠️"
    elif nmse < 1.0:
        return "Poor match", "❌"
    else:
        return "Very poor match (worse than noise)", "❌"

def main():
    parser = argparse.ArgumentParser(description='Validate model logits')
    parser.add_argument('-m', '--model-path', required=True,  help='Path to the model directory')
    args = parser.parse_args()

    model_name = os.path.basename(args.model_path)
    data_dir = Path("data")

    pytorch_file = data_dir / f"pytorch-{model_name}.bin"
    llamacpp_file = data_dir / f"llamacpp-{model_name}.bin"

    print(f"Model name: {model_name}")
    print(f"PyTorch logits file: {pytorch_file}")
    print(f"llama.cpp logits file: {llamacpp_file}")

    reference_file = pytorch_file
    test_file = llamacpp_file

    print("📊 NMSE Check for Model Comparison")
    print("=" * 50)
    print(f"Reference (ground truth): {reference_file}")
    print(f"Test (to evaluate):       {test_file}")
    print()

    try:
        print("Loading reference logits...")
        reference = load_logits(reference_file)
        print(f"  Shape: {reference.shape}, Type: f32")

        print("Loading test logits...")
        test = load_logits(test_file)
        print(f"  Shape: {test.shape}, Type: f32")

        # Check shapes match
        if reference.shape != test.shape:
            print(f"\n❌ Error: Shape mismatch!")
            print(f"  Reference: {reference.shape}")
            print(f"  Test: {test.shape}")
            sys.exit(1)

        print(f"\n✅ Shapes match: {reference.shape}")

        nmse_value, mse, ref_var = calculate_nmse(reference.data, test.data)

        # Additional metrics
        max_abs_error = max_abs_diff(reference.data, test.data)
        mean_abs_error = mean_abs_diff(reference.data, test.data)

        # Results
        print(f"\n📈 METRICS")
        print("=" * 30)
        print(f"MSE (Mean Squared Error):     {mse:.6e}")
        print(f"Reference Variance:           {ref_var:.6e}")
        print(f"NMSE:                         {nmse_value:.6e}")
        print(f"Max Absolute Error:           {max_abs_error:.6f}")
        print(f"Mean Absolute Error:          {mean_abs_error:.6f}")

        # NMSE in dB (common in signal processing)
        if nmse_value > 0:
            nmse_db = 10 * math.log10(nmse_value)
            print(f"NMSE (dB):                    {nmse_db:.2f} dB")

        # Interpretation
        interpretation, emoji = interpret_nmse(nmse_value)
        print(f"\n🎯 INTERPRETATION")
        print("=" * 30)
        print(f"{emoji} {interpretation}")

        # Detailed guidance
        print(f"\n📋 GUIDANCE")
        print("=" * 30)
        if nmse_value < 1e-3:
            print("✅ EXCELLENT: Your GGML conversion is working very well!")
            print("   The differences are negligible for practical use.")
        elif nmse_value < 1e-2:
            print("👍 GOOD: Your GGML conversion is working well.")
            print("   Small differences are likely due to precision/quantization.")
        elif nmse_value < 0.1:
            print("⚠️  ACCEPTABLE: Conversion is working but with some differences.")
            print("   Check if you're using quantization (Q4, Q8, etc.)")
            print("   Test generation quality to see if it's acceptable.")
        else:
            print("❌ PROBLEMATIC: Large differences detected.")
            print("   Check your conversion process for potential issues.")
            print("   Verify you're using the same model weights.")

        # NMSE benchmarks
        print(f"\n📚 NMSE BENCHMARKS")
        print("=" * 30)
        print("< 1e-6:  Essentially identical")
        print("< 1e-4:  Excellent (typical for good conversions)")
        print("< 1e-3:  Very good")
        print("< 1e-2:  Good (acceptable for most use cases)")
        print("< 0.1:   Acceptable (may need verification)")
        print("> 1.0:   Poor (worse than random)")

        # Exit code based on NMSE
        if nmse_value < 1e-2:
            print(f"\n✅ RESULT: PASS (NMSE = {nmse_value:.2e})")
            sys.exit(0)
        else:
            print(f"\n❌ RESULT: NEEDS REVIEW (NMSE = {nmse_value:.2e})")
            sys.exit(1)

    except Exception as e:
        print(f"❌ Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
