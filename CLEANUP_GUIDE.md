# Repository Cleanup Guide

## Overview
This document describes what files can be safely deleted from the repository to save space.

## Already Cleaned
✅ **`.ccache/`** - Build cache directory (2.2M) - Already removed

## Files That Can Be Safely Deleted

### Build Artifacts (Regenerated on Build)
- `build*/` - All build directories
- `*.o` - Object files
- `*.a` - Static libraries  
- `*.so` - Shared libraries
- `*.dll` - Windows dynamic libraries
- `*.exe` - Executables
- `*.bin` - Binary files
- `*.d` - Dependency files
- `compile_commands.json` - Clang compilation database

### Cache Directories (Regenerated)
- `.ccache/` - ccache build cache
- `.cache/` - General cache
- `__pycache__/` - Python bytecode cache
- `node_modules/` - Node.js dependencies (if present)

### Temporary Files
- `*.tmp` - Temporary files
- `*.log` - Log files
- `*.swp` - Vim swap files
- `tmp/` - Temporary directory

### Model Files (Large, Downloaded)
- `models/*.gguf` - Model files (can be re-downloaded)
- `*.gguf` - GGUF model files anywhere

## Files That Should NEVER Be Deleted

### Source Code
- `src/` - Main source code
- `examples/` - Example programs
- `common/` - Common utilities
- `include/` - Header files
- `ggml/` - GGML tensor library
- `rafaelia-baremetal/` - RAFAELIA baremetal module

### Documentation
- `*.md` - Markdown documentation
- `docs/` - Documentation directory
- `README.md` - Main readme
- `CONTRIBUTING.md` - Contribution guidelines

### Configuration Files
- `CMakeLists.txt` - Build configuration
- `.clang-format` - Code formatting rules
- `.gitignore` - Git ignore patterns
- `pyproject.toml` - Python project config
- `.pre-commit-config.yaml` - Pre-commit hooks

### Version Control
- `.git/` - Git repository data
- `.github/` - GitHub workflows and actions
- `.gitignore` - Git ignore patterns
- `.gitmodules` - Git submodule config

### Development Environment
- `.venv/` - Python virtual environment (keep for development)
- `.devops/` - DevOps configurations

## Automated Cleanup Script

To clean up build artifacts and temporary files:

```bash
#!/bin/bash
# Cleanup script for llamaRafaelia repository

echo "Cleaning build artifacts..."
rm -rf build* 2>/dev/null
rm -rf .ccache 2>/dev/null
rm -rf __pycache__ 2>/dev/null
rm -rf tmp/ 2>/dev/null

echo "Cleaning compiled files..."
find . -type f \( -name "*.o" -o -name "*.a" -o -name "*.so" -o -name "*.dll" \) -delete 2>/dev/null

echo "Cleaning temporary files..."
find . -type f \( -name "*.tmp" -o -name "*.log" -o -name "*.swp" \) -delete 2>/dev/null

echo "Cleanup complete!"
```

## Space Usage

After cleanup:
- Repository size: ~155M
- `.venv/` (Python env): ~29M
- `.git/` (version control): ~26M
- Source code and docs: ~100M

## .gitignore Configuration

The repository already has a comprehensive `.gitignore` file that excludes:
- Build artifacts (`build*`, `*.o`, `*.a`, `*.so`)
- IDE files (`.vscode/`, `.idea/`)
- Cache directories (`.ccache/`, `__pycache__/`)
- Temporary files (`*.tmp`, `*.log`)
- Model files (`*.gguf`)

This ensures these files are never accidentally committed.

## Recommendations

1. **Regular Cleanup**: Run cleanup periodically to save disk space
2. **Before Commits**: Ensure no build artifacts are staged
3. **After Builds**: Clean up if not actively developing
4. **CI/CD**: Automated builds should clean up after themselves

## Questions About Deletion?

If you're unsure whether a file can be deleted:
1. Check if it's in `.gitignore` (likely safe to delete)
2. Check if it's in `build*/` directories (safe to delete)
3. Check if it has extensions `.o`, `.a`, `.so`, `.tmp` (safe to delete)
4. When in doubt, DON'T delete - ask first!

## Contact

For questions about this cleanup guide, refer to the repository maintainer.
