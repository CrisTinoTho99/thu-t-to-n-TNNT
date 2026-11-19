"""Compatibility entry point for the shared H?i An model generator."""
from pathlib import Path
import runpy
import sys

source = Path(__file__).resolve().parents[1] / "src"
sys.path.insert(0, str(source))
try:
    generator = runpy.run_path(str(source / "build.py"))
finally:
    sys.path.pop(0)

build_scene = generator["build_scene"]
main = generator["main"]

if __name__ == "__main__":
    main()
