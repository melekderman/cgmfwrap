# cgmfwrap

Python bindings for generating single CGMF fission events.

## Requirements

- Python 3.8+
- CMake 3.15+
- A C++17 compiler and `git` (for the auto-fetch path)
- pybind11 — installed automatically as a build dependency by pip

CGMF itself does not need to be pre-installed. By default, the build clones
[lanl/CGMF](https://github.com/lanl/CGMF), compiles it as a static library
in-tree, and bundles the CGMF data files into the installed wheel. The
resulting `cgmfwrap` import has no external runtime dependencies and works
out of the box without setting any environment variable.

## Install

Default (auto-fetch CGMF from GitHub):

```bash
python3 -m pip install .
```

Using an existing CGMF source checkout instead of fetching:

```bash
python3 -m pip install . --config-settings=cmake.define.CGMF_ROOT=/path/to/CGMF
```

Pinning a CGMF revision (only meaningful with the auto-fetch path):

```bash
python3 -m pip install . \
  --config-settings=cmake.define.CGMF_GIT_TAG=v1.1.0
```

## Usage

```python
import cgmfwrap

event = cgmfwrap.run_event(92235, 2.0)  # ZAID, incident energy in MeV
print(event.nu_n, event.neutron_energies)
```

## Runtime Data Path

The bundled CGMF data lives next to the installed package
(`<site-packages>/cgmfwrap/data/`) and is selected automatically. To point
CGMF at a different data set, set CGMF's standard `CGMFDATA` environment
variable before the first call:

```bash
export CGMFDATA=/path/to/custom/CGMF/data
```
