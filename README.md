# 3D Road Map

A real-time **3D map viewer and route planner** built from scratch in C++ / OpenGL. Roads, buildings and points of interest are streamed from an OpenStreetMap-derived SQLite database, rendered as a 3D city with level-of-detail control and switchable map themes — and you can plan routes with A* by clicking two points on the map.

https://github.com/user-attachments/assets/d521d820-3686-4673-883c-926b73138ae9

*Latest demo (phase 9) — routing, buildings, POIs and theme switching. [Watch directly](video%20demo/phase%209.mp4) if the player doesn't render.*

## Features

- **Real-world city data** — an OpenStreetMap-derived SQLite database (bundled, RTree-indexed) covering Singapore
- **3D road rendering** — roads drawn as ribbons with widths, colors and visibility per highway type (motorway → footpath), LOD-filtered by camera altitude
- **Extruded buildings** — buildings polygonized from OSM footprints and extruded with type-based heights and colors (hospitals, schools, apartments, …), shown below ~350 m altitude
- **Points of interest** — categorized POIs rendered below ~100 m altitude with per-category colors
- **Spatial region caching** — the database is queried in bounding-box chunks and only re-queried when the camera leaves the cached region
- **A\* route planning** — click a start and goal point; clicks snap to the nearest road node via a KD-tree, and the route is drawn as a highlighted ribbon using A* with haversine-weighted edges
- **6 map themes** — Google / OSM / HERE in light & dark, loaded from `themes.json` and hot-swappable at runtime, each with its own sun and ambient settings
- **Dynamic lighting & skybox** — adjustable sun azimuth drives scene lighting
- **Debug HUD** — camera position, speed, segment/building/POI counts and query count shown in the window title

## Controls

| Input | Action |
| --- | --- |
| `W` / `A` / `S` / `D` | Pan forward / left / back / right |
| `Q` / `E` | Descend / ascend |
| Right mouse (hold) | Look around |
| Left click | Pick route start, then route goal |
| `-` / `=` | Decrease / increase camera speed |
| `[` / `]` | Rotate sun (change lighting direction) |
| `T` + `1`–`6` | Switch map theme |
| `ESC` | Quit |

## Tech Stack

- **C++20** with **OpenGL** (GLEW) and **GLFW**
- **EnTT** — entity-component-system for scene entities
- **SQLite3** (amalgamation, RTree extension enabled) for map data
- **GLM** for math, **nlohmann/json** for theme definitions
- **CMake** with `FetchContent` — all dependencies download automatically at configure time

## Building

Requirements: Windows, CMake 3.13+, a C++20 compiler (developed with Visual Studio), `git` and an internet connection (dependency download).

### Option 1 — batch script

```bat
run.bat
```

Configures and builds both Debug and Release into `build/`.

### Option 2 — manual

```bat
cmake -B build -S .
cmake --build build --config Release
```

### Running

Run the executable **from inside the `build/` directory** so relative data paths resolve:

```bat
cd build
Debug\jiaming.yeo-project-phase9.exe
```

## Project Structure

```
projects/jiaming.yeo-project-phase9/
├── include/
│   ├── Component/CameraComponent.hpp   # free-fly camera
│   ├── RoadGraph.hpp                    # graph + A* + KD-tree
│   ├── MapRenderer.hpp                  # road/building/POI/route rendering
│   ├── TestScene.hpp                    # scene, spatial caching, click handling
│   ├── ThemeManager.hpp / ColorTheme.hpp
│   ├── Skybox.hpp / CoordinateConverter.hpp / Window.hpp / ...
├── shaders/                             # GLSL vertex/fragment + skybox shaders
├── src/                                 # implementations + main.cpp
├── themes.json                          # the 6 map themes
└── map_extended.db                      # OSM-derived map data (bundled)
```

## Development Progression

| Phase | Demo |
| --- | --- |
| 1 | [screenshot](video%20demo/phase%201.png) |
| 2 | [screenshot](video%20demo/phase%202.png) |
| 3 | [gif](video%20demo/phase%203.gif) |
| 4 | [video](video%20demo/phase%204.mp4) |
| 5 | [gif](video%20demo/phase%205.gif) |
| 6 | [video](video%20demo/phase%206.mp4) |
| 7 | [video](video%20demo/phase%207.mp4) |
| 8 | [video](video%20demo/phase%208.mp4) |
| 9 | [video](video%20demo/phase%209.mp4) |

## Data

`map_extended.db` was converted from an [OpenStreetMap](https://www.openstreetmap.org) extract of Singapore.

Data © [OpenStreetMap contributors](https://www.openstreetmap.org/copyright), licensed under the [Open Database License (ODbL) v1.0](https://opendatacommons.org/licenses/odbl/).

## Acknowledgments

Course project for CSD-3151, DigiPen Institute of Technology Singapore.

Copyright © 2026 DigiPen Institute of Technology.
