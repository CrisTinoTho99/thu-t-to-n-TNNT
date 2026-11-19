# Virtual Landmark Tour

Shared C++17 / OpenGL 3.3 application for Eiffel Tower, Statue of Liberty, and
Elizabeth Tower (Big Ben) map packages.

## Build and test

Requirements: CMake 3.22+, Git, and either Visual Studio with the C++ workload
or MinGW-w64 with Ninja. The first CMake configure fetches pinned dependencies
and requires an internet connection.

MinGW-w64 + Ninja:

```powershell
cmake -S . -B build-mingw -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build-mingw --parallel 8
ctest --test-dir build-mingw --output-on-failure
.\build-mingw\bin\vlt.exe
```

For Visual Studio, configure with the installed generator shown by
`cmake --help`, then run `cmake --build build-vs --config Debug` and
`ctest --test-dir build-vs -C Debug --output-on-failure`.

## Map package ownership

Each map is an independent content package:

| Owner | Only edit this package |
|---|---|
| Project owner | `assets/maps/eiffel/` |
| Map owner 2 | `assets/maps/liberty/` |
| Map owner 3 | `assets/maps/bigben/` |

A package contains `map.json` and its own `models/` and `textures/`. The shared
menu discovers the three project packages; they all use `MapScene` rather than
per-map scene classes.
Keep model units in metres, Y up, and -Z forward. Export as GLB and ensure every
`parts[].node` name exactly matches one node in the GLB. Record the source and
license for external models/textures in a `SOURCES.md` inside the map package.

Do not edit `src/`, `assets/common/`, `CMakeLists.txt`, or the shared map schema
as part of a map task. Propose shared changes to the project owner instead.
Use one branch per map (`map/liberty`, `map/bigben`) and submit a pull request
to `main`; do not commit map work directly to `main`.

The detailed design and schema files are shared separately and intentionally
are not tracked in this repository. Ask the project owner for those before
building the final map package.
