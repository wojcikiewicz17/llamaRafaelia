#!/usr/bin/env python3

from __future__ import annotations

import array
import ast
import math
import struct
from dataclasses import dataclass
from pathlib import Path
from typing import Sequence

_NPY_MAGIC = b"\x93NUMPY"


@dataclass(frozen=True)
class F32Tensor:
    data: list[float]
    shape: tuple[int, ...]

    def reshape(self, *shape: int) -> "F32Tensor":
        if len(shape) == 1 and isinstance(shape[0], tuple):
            shape = shape[0]
        expected = 1
        for dim in shape:
            expected *= dim
        if expected != len(self.data):
            raise ValueError(f"Cannot reshape data of size {len(self.data)} into {shape}")
        return F32Tensor(self.data, tuple(shape))


def _fromfile_f32(path: Path) -> list[float]:
    size = path.stat().st_size
    if size % 4 != 0:
        raise ValueError(f"Binary file size must be multiple of 4 bytes: {path}")
    count = size // 4
    buf = array.array("f")
    with path.open("rb") as handle:
        buf.fromfile(handle, count)
    if buf.itemsize != 4:
        raise ValueError("Unexpected float size in array module")
    if struct.pack("=f", 1.0) != struct.pack("<f", 1.0):
        buf.byteswap()
    return buf.tolist()


def _parse_npy_header(handle) -> tuple[tuple[int, ...], str, bool]:
    header = handle.read(6)
    if header != _NPY_MAGIC:
        raise ValueError("Invalid NPY magic header")
    major, minor = handle.read(2)
    if major == 1:
        header_len = struct.unpack("<H", handle.read(2))[0]
    elif major == 2:
        header_len = struct.unpack("<I", handle.read(4))[0]
    else:
        raise ValueError(f"Unsupported NPY version: {major}.{minor}")
    header_bytes = handle.read(header_len)
    header_text = header_bytes.decode("latin1")
    header_dict = ast.literal_eval(header_text)
    shape = tuple(header_dict["shape"])
    dtype = header_dict["descr"]
    fortran_order = header_dict["fortran_order"]
    return shape, dtype, fortran_order


def _load_npy_f32(path: Path) -> F32Tensor:
    with path.open("rb") as handle:
        shape, dtype, fortran_order = _parse_npy_header(handle)
        if fortran_order:
            raise ValueError("Fortran-ordered NPY files are not supported")
        if dtype not in ("<f4", "|f4", "f4"):
            raise ValueError(f"Unsupported dtype in NPY file: {dtype}")
        count = 1
        for dim in shape:
            count *= dim
        raw = handle.read(count * 4)
        if len(raw) != count * 4:
            raise ValueError("NPY file is truncated")
        data = list(struct.unpack("<" + "f" * count, raw))
    return F32Tensor(data, shape)


def _load_text_f32(path: Path) -> list[float]:
    data: list[float] = []
    with path.open("r", encoding="utf-8") as handle:
        for line in handle:
            stripped = line.strip()
            if not stripped:
                continue
            if ":" in stripped:
                stripped = stripped.split(":", 1)[1].strip()
            data.append(float(stripped))
    return data


def load_f32(path: str | Path) -> F32Tensor:
    path = Path(path)
    if not path.exists():
        raise FileNotFoundError(f"File not found: {path}")
    if path.suffix == ".npy":
        return _load_npy_f32(path)
    if path.suffix == ".bin":
        data = _fromfile_f32(path)
        return F32Tensor(data, (len(data),))
    data = _load_text_f32(path)
    return F32Tensor(data, (len(data),))


def mean(values: Sequence[float]) -> float:
    if not values:
        return 0.0
    return sum(values) / len(values)


def variance(values: Sequence[float], mean_value: float | None = None) -> float:
    if not values:
        return 0.0
    if mean_value is None:
        mean_value = mean(values)
    return sum((v - mean_value) ** 2 for v in values) / len(values)


def nmse(reference: Sequence[float], test: Sequence[float]) -> tuple[float, float, float]:
    if len(reference) != len(test):
        raise ValueError("Reference and test lengths do not match")
    mse = mean([(t - r) ** 2 for r, t in zip(reference, test)])
    ref_var = variance(reference)
    if ref_var == 0:
        return (float("inf") if mse > 0 else 0.0), mse, ref_var
    return mse / ref_var, mse, ref_var


def max_abs_diff(reference: Sequence[float], test: Sequence[float]) -> float:
    return max(abs(t - r) for r, t in zip(reference, test))


def mean_abs_diff(reference: Sequence[float], test: Sequence[float]) -> float:
    return mean([abs(t - r) for r, t in zip(reference, test)])


def l2_norm(values: Sequence[float]) -> float:
    return math.sqrt(sum(v * v for v in values))


def cosine_similarity(a: Sequence[float], b: Sequence[float]) -> float:
    denom = l2_norm(a) * l2_norm(b)
    if denom == 0:
        return 0.0
    return sum(x * y for x, y in zip(a, b)) / denom


def cosine_similarity_matrix(rows: Sequence[Sequence[float]]) -> list[list[float]]:
    norms = [l2_norm(row) for row in rows]
    result: list[list[float]] = []
    for i, row in enumerate(rows):
        row_norm = norms[i] or 1e-8
        similarities = []
        for j, other in enumerate(rows):
            denom = row_norm * (norms[j] or 1e-8)
            similarities.append(sum(x * y for x, y in zip(row, other)) / denom)
        result.append(similarities)
    return result


def reshape_flat(values: Sequence[float], rows: int, cols: int) -> list[list[float]]:
    if rows * cols != len(values):
        raise ValueError("Cannot reshape values into requested rows/cols")
    return [list(values[i * cols:(i + 1) * cols]) for i in range(rows)]


def topk_indices(values: Sequence[float], k: int) -> list[int]:
    if k <= 0:
        return []
    indexed = sorted(enumerate(values), key=lambda pair: pair[1], reverse=True)
    return [idx for idx, _ in indexed[:k]]
