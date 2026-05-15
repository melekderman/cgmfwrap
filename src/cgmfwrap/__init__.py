"""Python bindings for generating single CGMF fission events."""

import os
from pathlib import Path

_PACKAGE_DIR = Path(__file__).resolve().parent
_DATA_DIR = _PACKAGE_DIR / "data"
if _DATA_DIR.is_dir():
    # Honor a user-supplied CGMFDATA, otherwise point CGMF at the data bundled
    # next to this package by `pip install`.
    os.environ.setdefault("CGMFDATA", str(_DATA_DIR))

from ._native import run_event, EventOutput  # noqa: E402, F401

__all__ = ["run_event", "EventOutput"]
